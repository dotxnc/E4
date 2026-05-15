/// E4 core: OpenGL rendering engine.
/// Manages a character-cell terminal buffer that is rendered via an
/// OpenGL shader to a full-screen quad.
const std = @import("std");
const c = @import("c.zig").c;

// The C shim provides gladLoadGL(glfwGetProcAddress) without type-cast noise.
extern fn e4_glad_load_gl() c_int;

// ---------------------------------------------------------------------------
// Embedded resources (replaces the ld -r -b binary makeres.sh approach)
// ---------------------------------------------------------------------------
const main_vs_src: [:0]const u8 = @embedFile("../../res/in/main_vs.glsl");
const main_fs_src: [:0]const u8 = @embedFile("../../res/in/main_fs.glsl");
const font_png: []const u8 = @embedFile("../../res/in/font.png");

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

/// Controls which color components of a cell are written when calling putChar.
pub const TileFilter = enum(u8) {
    none = 0,
    static = 1,
    background = 2,
    foreground = 3,
    both = 4,
};

/// An RGB palette entry (values 0–255).
pub const PaletteColor = struct { r: u8, g: u8, b: u8 };

/// A 2-D position in unsigned 16-bit coordinates.
pub const UVec16 = struct { x: u16, y: u16 };

/// Encode a background + foreground palette index into a single color byte.
/// Matches the C macro: MAKE_COLOR(b, f) = (b << 4) | f
pub inline fn makeColor(bg: u8, fg: u8) u8 {
    return (bg << 4) | fg;
}

/// Picture-in-Picture staging buffer.
pub const PipStage = struct {
    buffer: ?[]u32 = null,
    term_width: u16,
    term_height: u16,
    char_width: u16,
    char_height: u16,
};

// ---------------------------------------------------------------------------
// Module-level state (mirrors the C static globals)
// ---------------------------------------------------------------------------
var buffer: []u32 = &.{};
var term_w: u16 = 0;
var term_h: u16 = 0;
var char_w: u16 = 0;
var char_h: u16 = 0;
var font_w: i32 = 0;
var font_h: i32 = 0;
var screen_w: u16 = 0;
var screen_h: u16 = 0;
var cursor_x: u16 = 0;
var cursor_y: u16 = 0;

var filter_mode: TileFilter = .static;
var filter_stack: TileFilter = .none;

var vbo: c.GLuint = 0;
var ebo: c.GLuint = 0;
var shader_prog: c.GLuint = 0;
var font_tex: c.GLuint = 0;
var ssbo: c.GLuint = 0;

var u_data: c.GLint = 0;
var u_time: c.GLint = 0;
var u_char_size: c.GLint = 0;
var u_res: c.GLint = 0;
var u_tex_size: c.GLint = 0;
var u_buffer_size: c.GLint = 0;

var sci_x: u16 = 0;
var sci_y: u16 = 0;
var sci_w: u16 = 0;
var sci_h: u16 = 0;

var staged_buffer: ?[]u32 = null;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

fn compileShader(src: [:0]const u8, kind: c.GLenum) c.GLuint {
    const shader = c.glCreateShader(kind);
    const ptr: [*]const u8 = src.ptr;
    const len: c.GLint = @intCast(src.len);
    c.glShaderSource(shader, 1, &ptr, &len);
    c.glCompileShader(shader);

    var ok: c.GLint = 0;
    c.glGetShaderiv(shader, c.GL_COMPILE_STATUS, &ok);
    if (ok == c.GL_FALSE) {
        var log_len: c.GLint = 0;
        c.glGetShaderiv(shader, c.GL_INFO_LOG_LENGTH, &log_len);
        const log = std.heap.c_allocator.alloc(u8, @intCast(log_len)) catch unreachable;
        defer std.heap.c_allocator.free(log);
        c.glGetShaderInfoLog(shader, log_len, &log_len, log.ptr);
        std.debug.print("Shader compile error:\n{s}\n", .{log});
        std.process.exit(1);
    }
    return shader;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Load OpenGL function pointers (must be called after the GL context is current).
pub fn loadGl() void {
    if (e4_glad_load_gl() == 0) {
        std.debug.print("Failed to initialise OpenGL via glad.\n", .{});
        std.process.exit(1);
    }
}

/// Initialise the engine with the given terminal and screen dimensions.
pub fn init(
    tw: u16,
    th: u16,
    cw: u16,
    ch: u16,
    sw: u16,
    sh: u16,
) void {
    term_w = tw;
    term_h = th;
    char_w = cw;
    char_h = ch;
    screen_w = sw;
    screen_h = sh;

    // Allocate the cell buffer and zero it.
    buffer = std.heap.c_allocator.alloc(u32, @as(usize, tw) * th) catch unreachable;
    @memset(buffer, 0);

    sci_x = 0;
    sci_y = 0;
    sci_w = tw;
    sci_h = th;

    c.glViewport(0, 0, sw, sh);

    // ---- Screen-filling quad ----
    const Vertex = extern struct { pos: [2]f32, tex: [2]f32 };
    const vertices = [4]Vertex{
        .{ .pos = .{ -1, -1 }, .tex = .{ 0, 1 } },
        .{ .pos = .{ -1, 1 }, .tex = .{ 0, 0 } },
        .{ .pos = .{ 1, 1 }, .tex = .{ 1, 0 } },
        .{ .pos = .{ 1, -1 }, .tex = .{ 1, 1 } },
    };
    const indices = [6]c.GLuint{ 0, 1, 2, 0, 2, 3 };

    c.glGenBuffers(1, &vbo);
    c.glBindBuffer(c.GL_ARRAY_BUFFER, vbo);
    c.glBufferData(c.GL_ARRAY_BUFFER, @sizeOf(@TypeOf(vertices)), &vertices, c.GL_STATIC_DRAW);

    c.glGenBuffers(1, &ebo);
    c.glBindBuffer(c.GL_ELEMENT_ARRAY_BUFFER, ebo);
    c.glBufferData(c.GL_ELEMENT_ARRAY_BUFFER, @sizeOf(@TypeOf(indices)), &indices, c.GL_STATIC_DRAW);

    // ---- Compile and link shaders ----
    const vs = compileShader(main_vs_src, c.GL_VERTEX_SHADER);
    const fs = compileShader(main_fs_src, c.GL_FRAGMENT_SHADER);

    shader_prog = c.glCreateProgram();
    c.glAttachShader(shader_prog, vs);
    c.glAttachShader(shader_prog, fs);
    c.glLinkProgram(shader_prog);
    c.glUseProgram(shader_prog);
    c.glDeleteShader(vs);
    c.glDeleteShader(fs);

    // ---- Retrieve uniform / attribute locations ----
    u_data = c.glGetUniformLocation(shader_prog, "data");
    u_time = c.glGetUniformLocation(shader_prog, "time");
    u_char_size = c.glGetUniformLocation(shader_prog, "char_size");
    u_res = c.glGetUniformLocation(shader_prog, "res");
    u_tex_size = c.glGetUniformLocation(shader_prog, "tex_size");
    u_buffer_size = c.glGetUniformLocation(shader_prog, "buffer_size");

    const pos_loc: c.GLuint = @intCast(c.glGetAttribLocation(shader_prog, "position"));
    const tex_loc: c.GLuint = @intCast(c.glGetAttribLocation(shader_prog, "i_tex_coords"));
    const stride: c.GLsizei = @sizeOf(Vertex);
    c.glVertexAttribPointer(pos_loc, 2, c.GL_FLOAT, c.GL_FALSE, stride, null);
    c.glEnableVertexAttribArray(pos_loc);
    c.glVertexAttribPointer(
        tex_loc,
        2,
        c.GL_FLOAT,
        c.GL_FALSE,
        stride,
        @ptrFromInt(2 * @sizeOf(f32)),
    );
    c.glEnableVertexAttribArray(tex_loc);

    // ---- Load font texture via stb_image ----
    var img_w: c_int = 0;
    var img_h: c_int = 0;
    const font_pixels = c.stbi_load_from_memory(
        @ptrCast(font_png.ptr),
        @intCast(font_png.len),
        &img_w,
        &img_h,
        null,
        4,
    );
    font_w = img_w;
    font_h = img_h;

    c.glGenTextures(1, &font_tex);
    c.glBindTexture(c.GL_TEXTURE_2D, font_tex);
    c.glTexImage2D(
        c.GL_TEXTURE_2D,
        0,
        c.GL_RGBA,
        img_w,
        img_h,
        0,
        c.GL_RGBA,
        c.GL_UNSIGNED_BYTE,
        font_pixels,
    );
    c.glGenerateMipmap(c.GL_TEXTURE_2D);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MIN_FILTER, c.GL_NEAREST);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MAG_FILTER, c.GL_NEAREST);
    c.stbi_image_free(font_pixels);

    // ---- Create Shader Storage Buffer Object ----
    const buf_bytes: c.GLsizeiptr = @intCast(buffer.len * @sizeOf(u32));
    c.glGenBuffers(1, &ssbo);
    c.glBindBuffer(c.GL_SHADER_STORAGE_BUFFER, ssbo);
    c.glBufferData(c.GL_SHADER_STORAGE_BUFFER, buf_bytes, buffer.ptr, c.GL_DYNAMIC_COPY);
    c.glBindBufferBase(c.GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    // ---- Initial uniform values ----
    c.glUniform2f(u_char_size, @floatFromInt(cw), @floatFromInt(ch));
    c.glUniform2f(u_res, @floatFromInt(sw), @floatFromInt(sh));
    c.glUniform2f(u_tex_size, @floatFromInt(img_w), @floatFromInt(img_h));
    c.glUniform2f(u_buffer_size, @floatFromInt(tw), @floatFromInt(th));
}

/// Free all GPU resources and the cell buffer.
pub fn clean() void {
    std.heap.c_allocator.free(buffer);
    c.glDeleteBuffers(1, &vbo);
    c.glDeleteBuffers(1, &ebo);
    c.glDeleteBuffers(1, &ssbo);
    c.glDeleteProgram(shader_prog);
    c.glDeleteTextures(1, &font_tex);
}

/// Upload the cell buffer to the GPU and draw the full-screen quad.
pub fn render() void {
    c.glUniform2f(u_char_size, @floatFromInt(char_w), @floatFromInt(char_h));
    c.glUniform2f(u_res, @floatFromInt(screen_w), @floatFromInt(screen_h));
    c.glUniform2f(u_tex_size, @floatFromInt(font_w), @floatFromInt(font_h));
    c.glUniform2f(u_buffer_size, @floatFromInt(term_w), @floatFromInt(term_h));

    c.glBindBuffer(c.GL_ARRAY_BUFFER, vbo);
    c.glBindBuffer(c.GL_ELEMENT_ARRAY_BUFFER, ebo);
    c.glBindTexture(c.GL_TEXTURE_2D, font_tex);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MIN_FILTER, c.GL_NEAREST);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MAG_FILTER, c.GL_NEAREST);
    c.glBindBuffer(c.GL_SHADER_STORAGE_BUFFER, ssbo);

    c.glUniform1f(u_time, @floatCast(c.glfwGetTime()));

    const buf_bytes: c.GLsizeiptr = @intCast(buffer.len * @sizeOf(u32));
    c.glBufferData(c.GL_SHADER_STORAGE_BUFFER, buf_bytes, buffer.ptr, c.GL_DYNAMIC_DRAW);
    // u_data resolves to -1 (no "data" uniform; the shader uses an SSBO),
    // so this call is effectively a no-op but matches the original behaviour.
    c.glUniform1uiv(u_data, @intCast(buffer.len * @sizeOf(u32)), @ptrCast(buffer.ptr));

    c.glDrawElements(c.GL_TRIANGLES, 6, c.GL_UNSIGNED_INT, null);

    // Reset per-frame state.
    filter_stack = .none;
    sci_x = 0;
    sci_y = 0;
    sci_w = term_w;
    sci_h = term_h;
}

/// Set a scissor rectangle. Cells outside [x, x+w) × [y, y+h) will be ignored
/// by subsequent putChar calls.  Passing any zero dimension resets to full buffer.
pub fn setScissor(x: u16, y: u16, w: u16, h: u16) void {
    if (x == 0 or y == 0 or w == 0 or h == 0) {
        sci_x = 0;
        sci_y = 0;
        sci_w = term_w;
        sci_h = term_h;
        return;
    }
    sci_x = x;
    sci_y = y;
    sci_w = w;
    sci_h = h;
}

/// Write a character into the cell buffer with the given colour byte.
pub fn putChar(ch: u8, color: u8, x: u16, y: u16) void {
    if (x >= sci_x + sci_w or y >= sci_y + sci_h or x < sci_x or y < sci_y)
        return;

    const idx = @as(usize, y) * term_w + x;
    const cur_color: u8 = @intCast(buffer[idx] >> 8);

    const new_color: u8 = switch (filter_mode) {
        .static => color,
        .background => (cur_color & 0xF0) | color,
        .foreground => (color << 4) | (cur_color & 0x0F),
        .both => cur_color,
        .none => color,
    };

    buffer[idx] = (@as(u32, new_color) << 8) | ch;
}

/// Fill the entire cell buffer with the given character and colour.
pub fn clear(ch: u8, color: u8) void {
    const val: u32 = (@as(u32, color) << 8) | ch;
    @memset(buffer, val);
}

/// Return the character stored at the given cell position.
pub fn getChar(x: u16, y: u16) u8 {
    return @intCast(buffer[@as(usize, y) * term_w + x] & 0xFF);
}

pub fn charWidth() u16 {
    return char_w;
}
pub fn charHeight() u16 {
    return char_h;
}
pub fn width() u16 {
    return term_w;
}
pub fn height() u16 {
    return term_h;
}

/// Upload a 16-entry colour palette to the shader.
pub fn setPalette(palette: [16]PaletteColor) void {
    var name_buf: [12]u8 = undefined;
    for (palette, 0..) |col, i| {
        const name = std.fmt.bufPrintZ(&name_buf, "palette[{d}]", .{i}) catch unreachable;
        const loc = c.glGetUniformLocation(shader_prog, name.ptr);
        c.glUniform3f(
            loc,
            @as(f32, @floatFromInt(col.r)) / 255.0,
            @as(f32, @floatFromInt(col.g)) / 255.0,
            @as(f32, @floatFromInt(col.b)) / 255.0,
        );
    }
}

/// Update the cached cursor position from a raw pixel position.
pub fn setCursor(px: i32, py: i32) void {
    const term_res_w: u32 = @as(u32, term_w) * char_w;
    const term_res_h: u32 = @as(u32, term_h) * char_h;
    const scale_x: f32 = @as(f32, @floatFromInt(screen_w)) / @as(f32, @floatFromInt(term_res_w));
    const scale_y: f32 = @as(f32, @floatFromInt(screen_h)) / @as(f32, @floatFromInt(term_res_h));
    cursor_x = @intFromFloat(@as(f32, @floatFromInt(px)) / @as(f32, @floatFromInt(char_w)) / scale_x);
    cursor_y = @intFromFloat(@as(f32, @floatFromInt(py)) / @as(f32, @floatFromInt(char_h)) / scale_y);
}

/// Return the cursor position in cell coordinates.
pub fn cursor() UVec16 {
    return .{ .x = cursor_x, .y = cursor_y };
}

/// Set the active tile filter (controls which colour channels are written).
pub fn setFilter(f: TileFilter) void {
    filter_mode = f;
}

/// Return the active tile filter.
pub fn getFilter() TileFilter {
    return filter_mode;
}

/// Save the current tile filter onto a one-level stack.
pub fn pushFilter() void {
    filter_stack = filter_mode;
}

/// Restore the tile filter previously saved with pushFilter.
pub fn popFilter() void {
    filter_mode = filter_stack;
    filter_stack = .none;
}

// Deprecated aliases kept for compatibility.
pub const setMode = setFilter;
pub const getMode = getFilter;
pub const pushMode = pushFilter;
pub const popMode = popFilter;

/// Redirect buffer writes to a PipStage's off-screen buffer.
pub fn pipStage(stage: *PipStage) void {
    if (stage.buffer == null) {
        stage.buffer = std.heap.c_allocator.alloc(
            u32,
            @as(usize, stage.term_width) * stage.term_height,
        ) catch unreachable;
    }
    staged_buffer = buffer;
    buffer = stage.buffer.?;
}

/// Restore the main buffer after a pipStage call and upload the stage
/// uniform values so the next render() call uses the stage dimensions.
pub fn pipDraw() void {
    c.glUniform2f(u_char_size, @floatFromInt(char_w), @floatFromInt(char_h));
    c.glUniform2f(u_res, @floatFromInt(screen_w), @floatFromInt(screen_h));
    c.glUniform2f(u_tex_size, @floatFromInt(font_w), @floatFromInt(font_h));
    c.glUniform2f(u_buffer_size, @floatFromInt(term_w), @floatFromInt(term_h));
}
