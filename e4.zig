/// E4 library root module.
/// This file sits at the project root so that the package directory equals the
/// project root, allowing @embedFile in sub-modules to reach res/ without
/// violating the "file outside package path" constraint.
pub const bootstrap = @import("src/e4/bootstrap.zig");
pub const core = @import("src/e4/core.zig");
pub const draw = @import("src/e4/draw.zig");
pub const gui = @import("src/e4/gui.zig");
pub const scene = @import("src/e4/scene.zig");
