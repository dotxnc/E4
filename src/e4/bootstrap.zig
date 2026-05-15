/// E4 bootstrap: GLFW window creation and main loop.
/// Mirrors bootstrap.c / bootstrap.h from the original C implementation.
const std = @import("std");
const c = @import("c.zig").c;
const core = @import("core.zig");

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

pub const InputAction = enum(i32) {
    release = 0,
    press = 1,
    repeat = 2,
    _,
};

pub const InputMod = enum(i32) {
    none = 0,
    shift = 1,
    control = 2,
    alt = 4,
    super = 8,
    _,
};

pub const MouseButton = enum(i32) {
    left = 0,
    right = 1,
    middle = 2,
    _,
};

pub const InputKey = enum(i32) {
    unknown = -1,
    space = 32,
    apostrophe = 39,
    comma = 44,
    minus = 45,
    period = 46,
    slash = 47,
    @"0" = 48,
    @"1" = 49,
    @"2" = 50,
    @"3" = 51,
    @"4" = 52,
    @"5" = 53,
    @"6" = 54,
    @"7" = 55,
    @"8" = 56,
    @"9" = 57,
    semicolon = 59,
    equal = 61,
    a = 65,
    b = 66,
    c_key = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,
    left_bracket = 91,
    backslash = 92,
    right_bracket = 93,
    grave = 96,
    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,
    insert = 260,
    delete = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    page_up = 266,
    page_down = 267,
    home = 268,
    end = 269,
    caps_lock = 280,
    scroll_lock = 281,
    num_lock = 282,
    print_screen = 283,
    pause = 284,
    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    f13 = 302,
    f14 = 303,
    f15 = 304,
    f16 = 305,
    f17 = 306,
    f18 = 307,
    f19 = 308,
    f20 = 309,
    f21 = 310,
    f22 = 311,
    f23 = 312,
    f24 = 313,
    f25 = 314,
    kp0 = 320,
    kp1 = 321,
    kp2 = 322,
    kp3 = 323,
    kp4 = 324,
    kp5 = 325,
    kp6 = 326,
    kp7 = 327,
    kp8 = 328,
    kp9 = 329,
    kp_decimal = 330,
    kp_divide = 331,
    kp_multiply = 332,
    kp_subtract = 333,
    kp_add = 334,
    kp_enter = 335,
    kp_equal = 336,
    left_shift = 340,
    left_control = 341,
    left_alt = 342,
    left_super = 343,
    right_shift = 344,
    right_control = 345,
    right_alt = 346,
    right_super = 347,
    menu = 348,
    _,
};

/// Callback signatures — users fill one of these per event type.
pub const InitFn = *const fn () void;
pub const UpdateFn = *const fn (dt: f32) void;
pub const DrawFn = *const fn (dt: f32) void;
pub const ClickFn = *const fn (x: i32, y: i32, action: InputAction, button: MouseButton) void;
pub const KeyFn = *const fn (key: i32, action: InputAction, mod: InputMod) void;

pub const Callbacks = struct {
    init: InitFn,
    update: UpdateFn,
    draw: DrawFn,
    click: ClickFn,
    key: KeyFn,
};

pub const WindowSettings = struct {
    title: [*:0]const u8,
    width: i32,
    height: i32,
    callbacks: Callbacks,
};

pub const EngineSettings = struct {
    term_width: u16,
    term_height: u16,
    char_width: u16,
    char_height: u16,
};

// ---------------------------------------------------------------------------
// Module state
// ---------------------------------------------------------------------------

/// Maximum GLFW key code value (GLFW_KEY_MENU = 348). We allocate 512 slots
/// to have a safe margin without wasting much memory.
const KEY_SLOTS: usize = 512;
const KEY_NONE: i32 = -1;

var window: ?*c.GLFWwindow = null;
var win_settings: WindowSettings = undefined;
var timer: f32 = 0.0;
var delta_time: f32 = 0.0;
var keycheck: [KEY_SLOTS]i32 = [_]i32{KEY_NONE} ** KEY_SLOTS;
var mousecheck: [3]i32 = [_]i32{KEY_NONE} ** 3;
var char_pressed: u32 = 0;

// ---------------------------------------------------------------------------
// GLFW callbacks (must have C calling convention)
// ---------------------------------------------------------------------------

fn keyCallback(
    _win: ?*c.GLFWwindow,
    key: c_int,
    _scan: c_int,
    action: c_int,
    mods: c_int,
) callconv(.c) void {
    _ = _win;
    _ = _scan;
    win_settings.callbacks.key(key, @enumFromInt(action), @enumFromInt(mods));
    if (key >= 0 and key < KEY_SLOTS) {
        keycheck[@intCast(key)] = action;
    }
}

fn clickCallback(
    win: ?*c.GLFWwindow,
    button: c_int,
    action: c_int,
    _mods: c_int,
) callconv(.c) void {
    _ = _mods;
    var x: f64 = 0;
    var y: f64 = 0;
    c.glfwGetCursorPos(win, &x, &y);
    win_settings.callbacks.click(
        @intFromFloat(x),
        @intFromFloat(y),
        @enumFromInt(action),
        @enumFromInt(button),
    );
    if (button >= 0 and button < 3) {
        mousecheck[@intCast(button)] = action;
    }
}

fn charCallback(_win: ?*c.GLFWwindow, codepoint: c_uint) callconv(.c) void {
    _ = _win;
    char_pressed = codepoint;
}

fn cursorCallback(_win: ?*c.GLFWwindow, x: f64, y: f64) callconv(.c) void {
    _ = _win;
    core.setCursor(@intFromFloat(x), @intFromFloat(y));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Create the OpenGL window and initialise the E4 engine.
pub fn init(ws: WindowSettings, es: EngineSettings) void {
    if (c.glfwInit() == 0) return;

    c.glfwWindowHint(c.GLFW_RESIZABLE, c.GLFW_FALSE);
    window = c.glfwCreateWindow(ws.width, ws.height, ws.title, null, null);
    if (window == null) {
        c.glfwTerminate();
        return;
    }

    win_settings = ws;

    _ = c.glfwSetMouseButtonCallback(window, clickCallback);
    _ = c.glfwSetKeyCallback(window, keyCallback);
    _ = c.glfwSetCharCallback(window, charCallback);
    _ = c.glfwSetCursorPosCallback(window, cursorCallback);
    c.glfwMakeContextCurrent(window);

    core.loadGl();
    core.init(
        es.term_width,
        es.term_height,
        es.char_width,
        es.char_height,
        @intCast(ws.width),
        @intCast(ws.height),
    );

    ws.callbacks.init();

    @memset(&keycheck, KEY_NONE);
    @memset(&mousecheck, KEY_NONE);
}

/// Run the main render / update loop until the window is closed.
pub fn run() void {
    var time_last: f32 = @floatCast(c.glfwGetTime());
    timer = time_last;

    while (c.glfwWindowShouldClose(window) == 0) {
        const time_now: f32 = @floatCast(c.glfwGetTime());
        const dt = time_now - time_last;
        time_last = time_now;
        timer = time_now;
        delta_time = dt;

        char_pressed = 0;
        c.glfwPollEvents();

        win_settings.callbacks.update(dt);
        win_settings.callbacks.draw(dt);

        c.glClearColor(0, 0, 0, 1);
        c.glClear(c.GL_COLOR_BUFFER_BIT);
        core.render();
        c.glfwSwapBuffers(window);
    }

    core.clean();
    c.glfwDestroyWindow(window);
}

/// Signal the main loop to exit on the next iteration.
pub fn stop() void {
    c.glfwSetWindowShouldClose(window, c.GLFW_TRUE);
}

/// Elapsed time in seconds since the start of the loop.
pub fn time() f32 {
    return timer;
}

/// Frame delta-time in seconds.
pub fn dtime() f32 {
    return delta_time;
}

/// Returns true once when the key is first pressed; clears the flag.
pub fn keyPressed(key: InputKey) bool {
    const k = @intFromEnum(key);
    if (k < 0 or k >= KEY_SLOTS) return false;
    const idx: usize = @intCast(k);
    if (keycheck[idx] == c.GLFW_PRESS) {
        keycheck[idx] = KEY_NONE;
        return true;
    }
    return false;
}

/// Returns true once when the key is released; clears the flag.
pub fn keyReleased(key: InputKey) bool {
    const k = @intFromEnum(key);
    if (k < 0 or k >= KEY_SLOTS) return false;
    const idx: usize = @intCast(k);
    if (keycheck[idx] == c.GLFW_RELEASE) {
        keycheck[idx] = KEY_NONE;
        return true;
    }
    return false;
}

/// Returns true once per key-repeat event; clears the flag.
pub fn keyRepeat(key: InputKey) bool {
    const k = @intFromEnum(key);
    if (k < 0 or k >= KEY_SLOTS) return false;
    const idx: usize = @intCast(k);
    if (keycheck[idx] == c.GLFW_REPEAT) {
        keycheck[idx] = KEY_NONE;
        return true;
    }
    return false;
}

/// Returns true while the key is held down.
pub fn keyDown(key: InputKey) bool {
    return c.glfwGetKey(window, @intFromEnum(key)) == c.GLFW_PRESS;
}

/// Returns true once when the mouse button is first pressed; clears the flag.
pub fn mousePressed(button: MouseButton) bool {
    const b: usize = @intCast(@intFromEnum(button));
    if (mousecheck[b] == c.GLFW_PRESS) {
        mousecheck[b] = KEY_NONE;
        return true;
    }
    return false;
}

/// Returns true once when the mouse button is released; clears the flag.
pub fn mouseReleased(button: MouseButton) bool {
    const b: usize = @intCast(@intFromEnum(button));
    if (mousecheck[b] == c.GLFW_RELEASE) {
        mousecheck[b] = KEY_NONE;
        return true;
    }
    return false;
}

/// Returns true while the mouse button is held down.
pub fn mouseDown(button: MouseButton) bool {
    return c.glfwGetMouseButton(window, @intFromEnum(button)) != 0;
}

/// Return the most recently typed character (Unicode code point ≤ 0xFF).
/// Returns 0 if no character was typed this frame.
pub fn getChar() u8 {
    return @intCast(char_pressed & 0xFF);
}
