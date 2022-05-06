#include "e4core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "dep/stb_image.h"
#define GLAD_GL_IMPLEMENTATION
#include "dep/gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u16* _buffer;
static u32 _buffer_size;

static u16 _term_width;
static u16 _term_height;
static u16 _char_width;
static u16 _char_height;
static i32 _font_width;
static i32 _font_height;

static u32 _vbo;
static u32 _ebo;
static u32 _shader;
static u32 _font;
static u32 _ssbo;

static u32 _shader_time;
static u32 _shader_char_size;
static u32 _shader_res;
static u32 _shader_tex_size;
static u32 _shader_buffer_size;

extern u8 _main_vs_data[] asm("_binary_main_vs_glsl_start");
extern u8 _main_vs_size[] asm("_binary_main_vs_glsl_size");

extern u8 _main_fs_data[] asm("_binary_main_fs_glsl_start");
extern u8 _main_fs_size[] asm("_binary_main_fs_glsl_size");

extern u8 _font_data[] asm("_binary_font_png_start");
extern u8 _font_size[] asm("_binary_font_png_size");

void e4core_loadgl()
{
    gladLoadGL(glfwGetProcAddress);
}

void e4core_init(u16 term_width, u16 term_height, u16 char_width, u16 char_height, u16 screen_width, u16 screen_height)
{
    // Set variables and allocate memory
    _term_width = term_width;
    _term_height = term_height;
    _char_width = char_width;
    _char_height = char_height;

    _buffer = malloc(sizeof(u16) * term_width * term_height);
    _buffer_size = sizeof(u16) * term_width * term_height;
    for (u16 i = 0; i < term_width * term_height; i++)
        _buffer[i] = 0xff00;

    _buffer[20] = 0x9f05;
    // Vertex attributes
    struct Vertex
    {
        f32 _position[2];
        f32 _tex_coords[2];
    };

    // Screen quad vertices
    struct Vertex vertices[4] = {
        (struct Vertex){{-1, -1}, {0, 1}},
        (struct Vertex){{-1, 1}, {0, 0}},
        (struct Vertex){{1, 1}, {1, 0}},
        (struct Vertex){{1, -1}, {1, 1}},
    };
    u32 indices[6] = {
        0,
        1,
        2,
        0,
        2,
        3,
    };

    // Generate the screen quad buffers
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * 4, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * 6, indices, GL_STATIC_DRAW);

    // Compile shaders
    u32 success = 0;
    u8 error[512];
    i32 nul = 0;

    i32 vs_sz = (i32)((void*)_main_vs_size);
    char* vs_sc = malloc(sizeof(char) * vs_sz + 1);
    memcpy(vs_sc, _main_vs_data, (unsigned long)vs_sz);
    vs_sc[vs_sz] = '\0';

    u32 vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, (const char**)&vs_sc, NULL);
    glCompileShader(vs_shader);

    // TODO: error checking

    i32 fs_sz = (i32)((void*)_main_fs_size);
    char* fs_sc = malloc(sizeof(char) * fs_sz + 1);
    memcpy(fs_sc, _main_fs_data, (unsigned long)fs_sz);
    fs_sc[fs_sz] = '\0';

    u32 fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, (const char**)&fs_sc, NULL);
    glCompileShader(fs_shader);

    // TODO: error checking

    // Link the shader program
    _shader = glCreateProgram();
    glAttachShader(_shader, vs_shader);
    glAttachShader(_shader, fs_shader);
    glLinkProgram(_shader);
    glUseProgram(_shader);

    // Don't need shader binary anymore
    glDeleteShader(vs_shader);
    glDeleteShader(fs_shader);

    // Retrieve shader uniform locations
    _shader_time = glGetUniformLocation(_shader, "time");
    _shader_char_size = glGetUniformLocation(_shader, "char_size");
    _shader_res = glGetUniformLocation(_shader, "res");
    _shader_tex_size = glGetUniformLocation(_shader, "tex_size");
    _shader_buffer_size = glGetUniformLocation(_shader, "buffer_size");

    // Set vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    // Load font text
    i32 font_sz = (i32)((void*)_font_size);
    u8* font_raw = malloc(sizeof(u8) * font_sz);
    memcpy(font_raw, _font_data, font_sz);
    u8* font = stbi_load_from_memory(font_raw, font_sz, &_font_width, &_font_height, NULL, 4);

    // Upload font to gpu
    glGenTextures(1, &_font);
    glBindTexture(GL_TEXTURE_2D, _font);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _font_width, _font_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate shader storage buffr
    glGenBuffers(1, &_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _buffer_size, _buffer, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _ssbo);

    // Set shader uniform values
    glUniform2f(_shader_char_size, _char_width, _char_height);
    glUniform2f(_shader_res, screen_width, screen_height);
    glUniform2f(_shader_tex_size, _font_width, _font_height);
    glUniform2f(_shader_buffer_size, _term_width, _term_height);
}

void e4core_clean()
{
    free(_buffer);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteBuffers(1, &_ssbo);
    glDeleteProgram(_shader);
    glDeleteTextures(1, &_font);
}

void e4core_render()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glUseProgram(_shader);
    glBindTexture(GL_TEXTURE_2D, _font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);

    glUniform1f(_shader_time, glfwGetTime());

    glBufferData(GL_SHADER_STORAGE_BUFFER, _buffer_size, _buffer, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void e4core_putc(u8 ch, u8 color, u16 x, u16 y)
{
    _buffer[y * _term_width + x] = (color << 8) | ch;
}

void e4core_clear(u8 ch, u8 color)
{
    for (i32 i = 0; i < _term_width * _term_height; i++)
        _buffer[i] = (color << 8) | ch;
}
