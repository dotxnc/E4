#version 430

in vec2 position;
in vec2 i_tex_coords;
out vec2 tex_coords;

void main() {
    tex_coords = i_tex_coords;
    gl_Position = vec4(position, 0., 1.);
}
