/// E4 GUI widgets.
/// Provides immediate-mode-style widgets (button, drop-menu, textbox) backed
/// by a GuiContainer that holds a list of GuiObjects.
const std = @import("std");
const core = @import("core.zig");
const draw = @import("draw.zig");
const bootstrap = @import("bootstrap.zig");

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------

pub const GuiTheme = struct {
    button_normal: u8 = 0x70,
    button_hover: u8 = 0x7F,
    button_clicked: u8 = 0x7E,
    button_hotkey: u8 = 0x74,
    button_disabled: u8 = 0x78,

    window_border_high: u8 = 0x70,
    window_border_low: u8 = 0x78,
    window_background: u8 = 0x70,
    window_title: u8 = 0x7F,
    window_title_background: u8 = 0x70,

    text_outer: u8 = 0x70,
    text_cursor: u8 = 0x7F,
    text_text: u8 = 0x07,
    text_default_text: u8 = 0x78,
};

// ---------------------------------------------------------------------------
// GuiObject
// ---------------------------------------------------------------------------

pub const UpdateFn = *const fn (object: *GuiObject) void;
pub const DrawFn = *const fn (object: *GuiObject) void;

pub const GuiObject = struct {
    container: *GuiContainer,
    state: *anyopaque,
    _update: UpdateFn,
    _draw: DrawFn,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
    focused: bool = false,
    enabled: bool = true,
};

// ---------------------------------------------------------------------------
// GuiContainer
// ---------------------------------------------------------------------------

pub const GuiContainer = struct {
    theme: GuiTheme,
    objects: std.ArrayList(*GuiObject),

    pub fn create(allocator: std.mem.Allocator) !*GuiContainer {
        const container = try allocator.create(GuiContainer);
        container.* = .{
            .theme = .{},
            .objects = std.ArrayList(*GuiObject).init(allocator),
        };
        return container;
    }

    pub fn addObject(self: *GuiContainer, object: *GuiObject) void {
        object.container = self;
        self.objects.append(object) catch unreachable;
    }

    pub fn clean(self: *GuiContainer, allocator: std.mem.Allocator) void {
        for (self.objects.items) |obj| {
            allocator.destroy(obj);
        }
        self.objects.deinit();
        allocator.destroy(self);
    }

    pub fn update(self: *GuiContainer) void {
        for (self.objects.items) |obj| {
            obj._update(obj);
        }
    }

    pub fn draw(self: *GuiContainer) void {
        for (self.objects.items) |obj| {
            obj._draw(obj);
        }
    }
};

// ---------------------------------------------------------------------------
// Button widget
// ---------------------------------------------------------------------------

pub const ButtonCallback = *const fn (object: *GuiObject) void;

pub const ButtonState = struct {
    label: []const u8,
    callback: ?ButtonCallback,
};

fn buttonUpdate(object: *GuiObject) void {
    const state: *ButtonState = @ptrCast(@alignCast(object.state));
    const cur = core.cursor();
    const hover = cur.x >= object.x and
        cur.x < object.x + object.w and
        cur.y == object.y;
    if (hover and bootstrap.mousePressed(.left)) {
        if (state.callback) |cb| cb(object);
    }
}

fn buttonDraw(object: *GuiObject) void {
    const state: *ButtonState = @ptrCast(@alignCast(object.state));
    core.pushFilter();

    var color = object.container.theme.button_normal;
    const cur = core.cursor();
    const hover = cur.x >= object.x and
        cur.x < object.x + object.w and
        cur.y == object.y;

    if (hover and object.enabled) {
        color = object.container.theme.button_hover;
        if (bootstrap.mouseDown(.left))
            color = object.container.theme.button_clicked;
    }

    draw.text(state.label, color, object.x, object.y);
    core.popFilter();
}

pub fn createButton(
    allocator: std.mem.Allocator,
    label: []const u8,
    callback: ?ButtonCallback,
    x: u16,
    y: u16,
) !*GuiObject {
    const object = try allocator.create(GuiObject);
    const state = try allocator.create(ButtonState);
    state.* = .{ .label = label, .callback = callback };

    object.* = .{
        .container = undefined,
        .state = state,
        ._update = buttonUpdate,
        ._draw = buttonDraw,
        .x = x,
        .y = y,
        .w = @intCast(label.len),
        .h = 1,
    };
    return object;
}

// ---------------------------------------------------------------------------
// Drop-menu widget
// ---------------------------------------------------------------------------

pub const DropMenuCallback = *const fn (object: *GuiObject) void;

pub const DropMenuItem = struct {
    label: []const u8,
    callback: ?DropMenuCallback,
};

pub const DropMenuState = struct {
    label: []const u8,
    items: []const DropMenuItem,
    extended: bool = false,
};

fn dropMenuUpdate(object: *GuiObject) void {
    const state: *DropMenuState = @ptrCast(@alignCast(object.state));
    const cur = core.cursor();
    const hover = cur.x >= object.x and
        cur.x < object.x + object.w and
        cur.y == object.y;

    if (bootstrap.mousePressed(.left) and hover) {
        state.extended = !state.extended;
    }
}

fn dropMenuDraw(object: *GuiObject) void {
    const state: *DropMenuState = @ptrCast(@alignCast(object.state));
    core.pushFilter();

    var color = object.container.theme.button_normal;
    const cur = core.cursor();
    const hover = cur.x >= object.x and
        cur.x < object.x + object.w and
        cur.y == object.y;

    if (hover and object.enabled) {
        color = object.container.theme.button_hover;
        if (bootstrap.mouseDown(.left))
            color = object.container.theme.button_clicked;
    }

    draw.text(state.label, color, object.x, object.y);

    if (state.extended) {
        for (state.items, 0..) |item, idx| {
            var ic = object.container.theme.button_normal;
            const ic_hover = cur.x >= object.x and
                cur.x < object.x + @as(u16, @intCast(item.label.len)) and
                cur.y == object.y + 1 + @as(u16, @intCast(idx));

            if (ic_hover and object.enabled) {
                ic = object.container.theme.button_hover;
                if (bootstrap.mouseDown(.left))
                    ic = object.container.theme.button_clicked;
            }
            draw.text(item.label, ic, object.x, object.y + 1 + @as(u16, @intCast(idx)));
        }
    }

    core.popFilter();
}

pub fn createDropMenu(
    allocator: std.mem.Allocator,
    label: []const u8,
    items: []const DropMenuItem,
    x: u16,
    y: u16,
) !*GuiObject {
    const object = try allocator.create(GuiObject);
    const state = try allocator.create(DropMenuState);
    state.* = .{
        .label = label,
        .items = items,
        .extended = false,
    };

    object.* = .{
        .container = undefined,
        .state = state,
        ._update = dropMenuUpdate,
        ._draw = dropMenuDraw,
        .x = x,
        .y = y,
        .w = @intCast(label.len),
        .h = 1,
    };
    return object;
}

// ---------------------------------------------------------------------------
// Textbox widget
// ---------------------------------------------------------------------------

pub const TextboxState = struct {
    data: []u8,
    max_len: u8,
    len: u8 = 0,
    cursor: u8 = 0,
    blink: f32 = 0,
};

fn textboxUpdate(object: *GuiObject) void {
    const state: *TextboxState = @ptrCast(@alignCast(object.state));
    const ch = bootstrap.getChar();
    const can_write = state.len < state.max_len - 1;
    const is_empty = state.len == 0;

    if (ch > 0 and can_write) {
        state.len += 1;
        // Shift everything after the cursor one position to the right.
        const tail = state.max_len - state.cursor - 1;
        std.mem.copyBackwards(
            u8,
            state.data[state.cursor + 1 ..][0..tail],
            state.data[state.cursor..][0..tail],
        );
        state.data[state.cursor] = ch;
        state.cursor += 1;
        state.blink = 0;
    } else if ((bootstrap.keyPressed(.backspace) or bootstrap.keyRepeat(.backspace)) and
        !is_empty and state.cursor > 0)
    {
        const tail = state.max_len - state.cursor - 1;
        std.mem.copyForwards(
            u8,
            state.data[state.cursor - 1 ..][0..tail],
            state.data[state.cursor..][0..tail],
        );
        state.cursor -= 1;
        state.len -= 1;
        state.blink = 0;
    } else if ((bootstrap.keyPressed(.left) or bootstrap.keyRepeat(.left)) and state.cursor > 0) {
        state.cursor -= 1;
        state.blink = 0;
    } else if ((bootstrap.keyPressed(.right) or bootstrap.keyRepeat(.right)) and state.cursor < state.len) {
        state.cursor += 1;
        state.blink = 0;
    }

    state.blink += bootstrap.dtime();
}

fn textboxDraw(object: *GuiObject) void {
    const state: *TextboxState = @ptrCast(@alignCast(object.state));
    core.pushFilter();

    draw.text(state.data[0..state.len], object.container.theme.text_text, object.x, object.y);

    if (@mod(state.blink, 2.0) < 1.0) {
        core.putChar(0xdd, object.container.theme.text_cursor, object.x + state.cursor, object.y);
    }

    core.popFilter();
}

pub fn createTextbox(
    allocator: std.mem.Allocator,
    data: []u8,
    max_len: u8,
    x: u16,
    y: u16,
    width: u16,
) !*GuiObject {
    const object = try allocator.create(GuiObject);
    const state = try allocator.create(TextboxState);
    state.* = .{
        .data = data,
        .max_len = max_len,
    };
    @memset(data, 0);

    object.* = .{
        .container = undefined,
        .state = state,
        ._update = textboxUpdate,
        ._draw = textboxDraw,
        .x = x,
        .y = y,
        .w = width,
        .h = 1,
    };
    return object;
}
