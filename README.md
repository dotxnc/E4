# E4

A small terminal-emulator-style 2D engine that renders a character-cell
buffer through OpenGL.  The engine was originally written in C; this
branch is a full port to **Zig 0.16**.

---

## Building

### Prerequisites

| Dependency | Debian / Ubuntu | Notes |
|---|---|---|
| Zig 0.16 | [ziglang.org/download](https://ziglang.org/download/) | or via snap: `sudo snap install zig --classic --channel=latest/beta` |
| GLFW 3 | `sudo apt install libglfw3-dev` | headers + shared library |
| OpenGL | `sudo apt install libgl-dev` | usually already present |

> On other distros / macOS the package names will differ, but the same
> three components are needed.

### Compile

```sh
# Debug build (default)
zig build

# Optimised release build
zig build -Doptimize=ReleaseFast
```

Artifacts land in `zig-out/`:

| Path | What |
|---|---|
| `zig-out/lib/libe4.a` | E4 static library |
| `zig-out/bin/template` | Template / demo executable |

### Run the demo

```sh
zig build run
# or directly:
./zig-out/bin/template
```

The demo opens a 640×480 window and displays "Hello, E4!" bouncing
horizontally.  Press **Escape** or **F12** to quit.

---

## Repository layout

```
build.zig              Zig build script
e4.zig                 E4 library module root (package root for @embedFile)
src/
  e4/
    bootstrap.zig      GLFW window creation and main loop
    core.zig           OpenGL cell-buffer renderer
    draw.zig           Text and geometry drawing helpers
    gui.zig            Immediate-mode GUI widgets (button, drop-menu, textbox)
    scene.zig          Scene / state-machine management
    c.zig              Shared @cImport block (glad + GLFW + stb_image)
    root.zig           Re-export shim (kept for library-internal use)
  glad_impl.c          Compiles the glad OpenGL loader (header-only)
  stb_image_impl.c     Compiles stb_image (header-only)
  e4_cshim.c           C bridge for gladLoadGL(glfwGetProcAddress)
  dep/
    gl.h               glad 2 header-only OpenGL loader
    stb_image.h        stb_image single-file library
template/
  src/
    main.zig           Template / demo application
res/
  in/
    font.png           CP437 bitmap font (10×10 glyphs)
    main_vs.glsl       Vertex shader
    main_fs.glsl       Fragment shader (SSBO-based cell renderer)
```

---

## Using E4 in your own project

Add `e4.zig` (and the associated `src/`) to your project and reference it
from your `build.zig`:

```zig
const e4_mod = b.createModule(.{
    .root_source_file = b.path("path/to/e4.zig"),
    .target = target,
    .optimize = optimize,
});
e4_mod.addCSourceFiles(.{
    .files = &.{
        "path/to/src/glad_impl.c",
        "path/to/src/stb_image_impl.c",
        "path/to/src/e4_cshim.c",
    },
    .flags = &.{},
});
e4_mod.addIncludePath(b.path("path/to/src"));
e4_mod.linkSystemLibrary("glfw", .{});
e4_mod.linkSystemLibrary("GL", .{});
e4_mod.link_libc = true;

const e4_lib = b.addLibrary(.{ .name = "e4", .root_module = e4_mod });

// In your executable module:
your_mod.addImport("e4", e4_mod);
your_mod.linkLibrary(e4_lib);
```

Then in your Zig source:

```zig
const e4 = @import("e4");

pub fn main() void {
    e4.bootstrap.init(.{
        .title = "My App",
        .width = 640,
        .height = 480,
        .callbacks = .{
            .init   = onInit,
            .update = onUpdate,
            .draw   = onDraw,
            .click  = onClick,
            .key    = onKey,
        },
    }, .{
        .term_width  = 64,
        .term_height = 48,
        .char_width  = 10,
        .char_height = 10,
    });
    e4.bootstrap.run();
}
```

---

## Colour encoding

Colours are packed into a single `u8` as `(bg << 4) | fg`, where `bg`
and `fg` are indices (0–15) into the 16-entry palette.  The helper
`e4.core.makeColor(bg, fg)` performs this encoding.

The default palette matches the colour names in `template/src/main.zig`
(`Color.black = 0x0F`, `Color.green = 0x06`, etc.).

---

## Porting notes

* **Resources**: The original build used `ld -r -b binary` to link
  shader GLSL and font PNG files as binary blobs.  The Zig port uses
  `@embedFile` instead, which compiles the data directly into the binary
  and is fully cross-platform.

* **Allocator**: The Zig port uses `std.heap.c_allocator` for heap
  allocations to keep the code simple and to match the `malloc`/`free`
  pattern of the original.

* **Scene bug preserved**: `e4scene_update()` called `draw()` and
  `e4scene_draw()` called `update()` in the original C code (the bodies
  were swapped). `scene.zig` preserves this behaviour to avoid
  inadvertently breaking code that relied on it.

* **C-only build**: The original CMake / `makeres.sh` infrastructure is
  preserved as-is for reference but is no longer the primary build path.
  The Zig build (`zig build`) supersedes it.
