/// E4 drawing utilities.
/// Higher-level helpers built on top of core.putChar.
const std = @import("std");
const core = @import("core.zig");

pub const LineStyle = enum {
    single,
    double,
    single_vertical,
    single_horizontal,
};

/// Draw a null-terminated string at (x, y) with the given colour byte.
pub fn text(str: []const u8, color: u8, x: u16, y: u16) void {
    for (str, 0..) |ch, i| {
        core.putChar(ch, color, x + @as(u16, @intCast(i)), y);
    }
}

/// Draw up to `len` characters of the string.
pub fn textn(str: []const u8, color: u8, x: u16, y: u16, len: u16) void {
    const n = @min(len, @as(u16, @intCast(str.len)));
    for (str[0..n], 0..) |ch, i| {
        core.putChar(ch, color, x + @as(u16, @intCast(i)), y);
    }
}

/// Draw a formatted string (limited to 255 characters).
pub fn textf(color: u8, x: u16, y: u16, comptime fmt: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const s = std.fmt.bufPrint(&buf, fmt, args) catch {
        text("!ERROR!", 0xF0, x, y);
        return;
    };
    text(s, color, x, y);
}

/// Draw a box outline using CP437 line-drawing characters.
pub fn rect(x: u16, y: u16, w: u16, h: u16, color: u8, style: LineStyle) void {
    // Each style has 6 characters:
    //   [0] = top-left corner
    //   [1] = top-right corner
    //   [2] = bottom-right corner
    //   [3] = bottom-left corner
    //   [4] = horizontal bar
    //   [5] = vertical bar
    const palette: [6]u8 = switch (style) {
        .single => .{ 0xda, 0xbf, 0xd9, 0xc0, 0xc4, 0xb3 },
        .double => .{ 0xc9, 0xbb, 0xbc, 0xc8, 0xcd, 0xba },
        .single_vertical => .{ 0xd5, 0xb8, 0xbe, 0xd4, 0xcd, 0xb3 },
        .single_horizontal => .{ 0xd6, 0xb7, 0xbd, 0xd3, 0xc4, 0xba },
    };

    // Top and bottom edges
    var i: u16 = x;
    while (i < x + w) : (i += 1) {
        core.putChar(palette[4], color, i, y);
        core.putChar(palette[4], color, i, y + h - 1);
    }

    // Left and right edges
    var j: u16 = y;
    while (j < y + h) : (j += 1) {
        core.putChar(palette[5], color, x, j);
        core.putChar(palette[5], color, x + w - 1, j);
    }

    // Corners
    core.putChar(palette[0], color, x, y);
    core.putChar(palette[1], color, x + w - 1, y);
    core.putChar(palette[2], color, x + w - 1, y + h - 1);
    core.putChar(palette[3], color, x, y + h - 1);
}

/// Fill a rectangle with a repeating character.
pub fn fillRect(ch: u8, color: u8, x: u16, y: u16, w: u16, h: u16) void {
    var xx: u16 = x;
    while (xx < x + w) : (xx += 1) {
        var yy: u16 = y;
        while (yy < y + h) : (yy += 1) {
            core.putChar(ch, color, xx, yy);
        }
    }
}

/// Rich text placeholder (not yet implemented, matches the original stub).
pub fn richText(_str: []const u8, _x: u16, _y: u16) void {
    _ = _str;
    _ = _x;
    _ = _y;
}
