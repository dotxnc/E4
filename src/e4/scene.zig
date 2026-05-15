/// E4 scene management.
/// A SceneContainer groups init / update / draw callbacks for one "screen" of
/// the application; e4scene.change() hot-swaps the active scene.
const std = @import("std");

pub const SceneFn = *const fn () void;

pub const SceneContainer = struct {
    init: SceneFn,
    update: SceneFn,
    draw: SceneFn,
    uid: u32 = 0,
    initialized: bool = false,
};

var current_scene: SceneContainer = undefined;
var has_scene: bool = false;

/// Switch to a new scene, calling its init() the first time.
/// TODO: add "enter" and "leave" callbacks for cleaner scene transitions.
pub fn change(scene: SceneContainer) void {
    var s = scene;
    if (!s.initialized) {
        s.init();
        s.initialized = true;
    }
    current_scene = s;
    has_scene = true;
}

/// Return a copy of the currently active scene.
pub fn current() SceneContainer {
    return current_scene;
}

/// Dispatch the update callback of the active scene.
/// NOTE: the original C code has update() call draw() and vice-versa
/// (the function bodies are swapped), so this replicates that behaviour.
pub fn update() void {
    if (has_scene) current_scene.draw();
}

/// Dispatch the draw callback of the active scene.
/// NOTE: mirrors the body-swap in the original e4scene.c.
pub fn draw() void {
    if (has_scene) current_scene.update();
}
