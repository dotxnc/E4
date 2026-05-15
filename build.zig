// E4 Zig build configuration.
// Builds:
//   - libe4 (static library)  — src/e4/root.zig + C impl files
//   - template (executable)   — template/src/main.zig linking against libe4
//
// Requirements on the host:
//   sudo apt install libglfw3-dev libgl-dev   # Debian/Ubuntu
//   # or the equivalent for your distro / package manager
//
// Usage:
//   zig build            # build both targets
//   zig build run        # build & run the template demo
//   zig build -Doptimize=ReleaseFast run

const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // ---- E4 library module ------------------------------------------------
    const e4_mod = b.createModule(.{
        .root_source_file = b.path("e4.zig"),
        .target = target,
        .optimize = optimize,
    });

    // Compile the header-only C libraries (glad, stb_image) and the
    // glfwGetProcAddress bridge shim as part of the library.
    e4_mod.addCSourceFiles(.{
        .files = &.{
            "src/glad_impl.c",
            "src/stb_image_impl.c",
            "src/e4_cshim.c",
        },
        .flags = &.{},
    });

    // Add the src/ directory so that @cImport("dep/gl.h") etc. resolve correctly.
    e4_mod.addIncludePath(b.path("src"));

    // Link GLFW and OpenGL.
    e4_mod.linkSystemLibrary("glfw", .{});
    e4_mod.linkSystemLibrary("GL", .{});
    e4_mod.link_libc = true;

    // Produce an installable static library artifact.
    const e4_lib = b.addLibrary(.{
        .name = "e4",
        .root_module = e4_mod,
    });
    b.installArtifact(e4_lib);

    // ---- Template demo executable -----------------------------------------
    const demo_mod = b.createModule(.{
        .root_source_file = b.path("template/src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    // Make the "e4" module available as @import("e4") in the demo.
    demo_mod.addImport("e4", e4_mod);
    demo_mod.linkLibrary(e4_lib);
    demo_mod.link_libc = true;

    const demo = b.addExecutable(.{
        .name = "template",
        .root_module = demo_mod,
    });
    b.installArtifact(demo);

    // zig build run
    const run_cmd = b.addRunArtifact(demo);
    run_cmd.step.dependOn(b.getInstallStep());
    const run_step = b.step("run", "Build and run the template demo");
    run_step.dependOn(&run_cmd.step);
}
