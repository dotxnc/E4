/*
 * C shims for E4 Zig port.
 * These bridge cases where Zig's type system would require awkward casts
 * for calling C APIs (e.g. passing glfwGetProcAddress to gladLoadGL).
 */
#include "dep/gl.h"
#include <GLFW/glfw3.h>

/* Load all OpenGL function pointers via GLFW's proc-address getter. */
int e4_glad_load_gl(void)
{
    return gladLoadGL(glfwGetProcAddress);
}
