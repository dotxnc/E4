#include "e4core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "dep/stb_image.h"
#define GLAD_GL_IMPLEMENTATION
#include "dep/gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u16* _buffer = NULL;
static u32 _buffer_size;

static u16 _term_width;
static u16 _term_height;
static u16 _char_width;
static u16 _char_height;
static i32 _font_width;
static i32 _font_height;
static u16 _cursor_x;
static u16 _cursor_y;

static CopyModeE _mode = CopyMode_Static;
static CopyModeE _last_mode = CopyMode_None;

static u32 _vbo = 0;
static u32 _ebo = 0;
static u32 _shader = 0;
static u32 _font = 0;
static u32 _ssbo = 0;

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

static void GLAPIENTRY _e4core_openglmsg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

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
        _buffer[i] = 0x0000;

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

    // Compile vertex shader
    i32 vs_sz = (i32)((void*)_main_vs_size);
    char* vs_sc = malloc(sizeof(char) * vs_sz + 1);
    memcpy(vs_sc, _main_vs_data, (unsigned long)vs_sz);
    vs_sc[vs_sz] = '\0';
    printf("Vertex shader--\n\tSize: %d\n%s\\n---\n", vs_sz, vs_sc);

    u32 vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, (const char**)&vs_sc, NULL);
    glCompileShader(vs_shader);

    i32 success = 0;
    glGetShaderiv(vs_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        printf("Failed!\n");

        i32 err_len = 0;
        glGetShaderiv(vs_shader, GL_INFO_LOG_LENGTH, &err_len);

        char* buffer = malloc(sizeof(char) * err_len);
        glGetShaderInfoLog(vs_shader, err_len, &err_len, buffer);
        printf("%s\n", buffer);

        abort();
    }

    // Compile fragment shader
    i32 fs_sz = (i32)((void*)_main_fs_size);
    char* fs_sc = malloc(sizeof(char) * fs_sz + 1);
    memcpy(fs_sc, _main_fs_data, (unsigned long)fs_sz);
    fs_sc[fs_sz] = '\0';
    printf("Fragment shader--\n\tSize: %d\n%s\n---\n", fs_sz, fs_sc);

    u32 fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, (const char**)&fs_sc, NULL);
    glCompileShader(fs_shader);

    success = 0;
    glGetShaderiv(fs_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        printf("Failed!\n");

        i32 err_len = 0;
        glGetShaderiv(fs_shader, GL_INFO_LOG_LENGTH, &err_len);

        char* buffer = malloc(sizeof(char) * err_len);
        glGetShaderInfoLog(fs_shader, err_len, &err_len, buffer);
        printf("%s\n", buffer);

        abort();
    }

    // Link the shader program
    _shader = glCreateProgram();
    glAttachShader(_shader, vs_shader);
    glAttachShader(_shader, fs_shader);
    glLinkProgram(_shader);
    glUseProgram(_shader);

    // Don't need shader binary anymore
    glDeleteShader(vs_shader);
    glDeleteShader(fs_shader);
    free(vs_sc);
    free(fs_sc);

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

    // Generate shader storage buffer
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
    //    glUseProgram(_shader);
    glBindTexture(GL_TEXTURE_2D, _font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);

    glUniform1f(_shader_time, glfwGetTime());

    glBufferData(GL_SHADER_STORAGE_BUFFER, _buffer_size, _buffer, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    _last_mode = CopyMode_None;
}

void e4core_putc(u8 ch, u8 color, u16 x, u16 y)
{
    // avoid writing out of bounds
    if (x >= _term_width || y >= _term_height)
        return;

    u8 cur_color = _buffer[y * _term_width + x] >> 8;
    u8 new_color;
    switch (_mode)
    {
    case CopyMode_Static:
    {
        new_color = color;
    }
    break;
    case CopyMode_Background:
    {
        new_color = (cur_color & 0xF0) | color;
    }
    break;
    case CopyMode_Foreground:
    {
        new_color = (color << 4) | (cur_color & 0x0F);
    }
    break;
    case CopyMode_Both:
    {
        new_color = cur_color;
    }
    break;
    default:
    {
        new_color = color;
    }
    break;
    }

    _buffer[y * _term_width + x] = (new_color << 8) | ch;
}

void e4core_clear(u8 ch, u8 color)
{

    for (i32 i = 0; i < _term_width * _term_height; i++)
        _buffer[i] = (color << 8) | ch;
}

u16 e4core_char_width()
{
    return _char_width;
}

u16 e4core_char_height()
{
    return _char_height;
}

u16 e4core_width()
{
    return _term_width;
}

u16 e4core_height()
{
    return _term_height;
}

void e4core_cursor(i32 x, i32 y)
{
    // TODO: clamp to 0..buffer max
    _cursor_x = x / _char_width;
    _cursor_y = y / _char_height;
}

UVec16T e4core_get_cursor()
{
    return (UVec16T){_cursor_x, _cursor_y};
}

void e4core_copy_mode(CopyModeE mode)
{
    _mode = mode;
}

CopyModeE e4core_get_mode()
{
    return _mode;
}

void e4core_push_mode()
{
    assert(_last_mode == CopyMode_None && "You must pop the current copy mode first.");
    _last_mode = _mode;
}

void e4core_pop_mode()
{
    assert(_last_mode != CopyMode_None && "You must push a copy mode first.");
    _mode = _last_mode;
    _last_mode = CopyMode_None;
}
