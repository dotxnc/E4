/// E4 template / demo application.
/// Mirrors template/src/main.c from the original C implementation.
const std = @import("std");
const e4 = @import("e4");
const math = std.math;

// Default colour palette indices (match the shader's built-in palette).
const Color = enum(u8) {
    red = 0x00,
    light_red,
    light_brown,
    brown,
    orange,
    yellow,
    green,
    light_green,
    white,
    light_blue,
    blue,
    dark_blue,
    light_grey,
    grey,
    purple,
    black = 0x0F,
};

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

pub fn main() void {
    const window_settings = e4.bootstrap.WindowSettings{
        .title = "Hello, E4",
        .width = 640,
        .height = 480,
        .callbacks = .{
            .init = onInit,
            .update = onUpdate,
            .draw = onDraw,
            .click = onClick,
            .key = onKey,
        },
    };

    const engine_settings = e4.bootstrap.EngineSettings{
        .term_width = 64,
        .term_height = 48,
        .char_width = 10,
        .char_height = 10,
    };

    e4.bootstrap.init(window_settings, engine_settings);
    e4.bootstrap.run();
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

fn onInit() void {}

fn onUpdate(_dt: f32) void {
    _ = _dt;
}

fn onDraw(_dt: f32) void {
    _ = _dt;
    const bg_color = @intFromEnum(Color.black);
    e4.core.clear(0x00, e4.core.makeColor(bg_color, bg_color));

    const x_offset: f32 = @floatCast(15.0 + math.sin(e4.bootstrap.time() * 2.0) * 8.0);
    e4.draw.text(
        "Hello, E4!",
        e4.core.makeColor(@intFromEnum(Color.black), @intFromEnum(Color.green)),
        @intFromFloat(x_offset),
        1,
    );
}

fn onClick(_x: i32, _y: i32, _action: e4.bootstrap.InputAction, _button: e4.bootstrap.MouseButton) void {
    _ = _x;
    _ = _y;
    _ = _action;
    _ = _button;
}

fn onKey(key: i32, _action: e4.bootstrap.InputAction, _mod: e4.bootstrap.InputMod) void {
    _ = _action;
    _ = _mod;
    // F12 or Escape closes the window.
    if (key == @intFromEnum(e4.bootstrap.InputKey.f12) or
        key == @intFromEnum(e4.bootstrap.InputKey.escape))
    {
        e4.bootstrap.stop();
    }
}
