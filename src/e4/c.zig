/// Shared C imports for the E4 library.
/// All Zig modules that need GLFW, OpenGL (glad), or stb_image import this.
pub const c = @cImport({
    // glad must come first so it defines the OpenGL header guards,
    // preventing GLFW from pulling in its own copy of the GL headers.
    @cInclude("dep/gl.h");
    @cInclude("GLFW/glfw3.h");
    @cInclude("dep/stb_image.h");
});
