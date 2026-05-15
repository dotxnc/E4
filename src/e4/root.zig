/// E4 library root module.
/// Re-export every public sub-module so callers can do:
///   const e4 = @import("e4");
///   e4.core.clear(...)
///   e4.bootstrap.run()
pub const bootstrap = @import("bootstrap.zig");
pub const core = @import("core.zig");
pub const draw = @import("draw.zig");
pub const gui = @import("gui.zig");
pub const scene = @import("scene.zig");
