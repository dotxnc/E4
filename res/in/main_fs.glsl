#version 450
//#extension GL_NV_gpu_shader5: enable

in vec2 tex_coords;
out vec4 color;

uniform sampler2D font_texture;

layout (std430, binding = 3) buffer SSBOBlock {
    uint data[];
};

uniform float time;

uniform vec2 char_size = vec2(10, 10);
uniform vec2 res = vec2(640, 480);
uniform vec2 tex_size = vec2(160, 160);
uniform vec2 buffer_size = vec2(64, 48);

uniform vec3 palette[16] = {
    vec3(0.549, 0.118, 0.173),
    vec3(0.863, 0.267, 0.235),
    vec3(1, 0.549, 0.4),
    vec3(0.78, 0.357, 0.22),
    vec3(0.839, 0.435, 0.141),
    vec3(0.894, 0.729, 0.196),
    vec3(0.129, 0.569, 0.231),
    vec3(0.514, 0.71, 0.208),
    vec3(0.922, 0.835, 0.741),
    vec3(0.4, 0.765, 0.851),
    vec3(0.22, 0.486, 0.933),
    vec3(0.208, 0.224, 0.635),
    vec3(0.6, 0.553, 0.635),
    vec3(0.349, 0.306, 0.435),
    vec3(0.169, 0.102, 0.294),
    vec3(0.031, 0.02, 0.055),
};

uint get_array(int x, int y)
{
    return data[ int(floor(buffer_size.x)) * y + x ];
}

void main()
{
    
    // Position in "buffer-space" of the current character/tile
    double x = (tex_coords.x * res.x)/int(char_size.x);
    double y = (tex_coords.y * res.y)/int(char_size.y);

    vec2 ts = tex_size;
    vec2 ls = ts/char_size;

    uint op = uint(get_array(int(floor(x)), int(floor(y))));
    uint c = uint(op&uint(0x00ff));

    // Position of the glyph on the font texture
    double ox = (double(c&0x0f)*char_size.x)/tex_size.x;
    double oy = (double(c>>4)*char_size.y)/tex_size.y;

    // 
    double fx = (fract(x))/ls.x;// * 16;
    double fy = (fract(y))/ls.y;// * 16;
    double gx = clamp(ox+fx, 0, 1);
    double gy = clamp(oy+fy, 0, 1);

    vec3 col = texture2D(font_texture, vec2(gx, gy)).rgb;

    uint co = uint(op>>8);
    uint fg = co&uint(0x0F);
    uint bg = (co&uint(0xF0))>>4;

    if (col.r > 0)
    {
        color.rgb = col * palette[fg];
    }
    else
    {
        color.rgb = palette[bg];
    }
    
    // double lin_x = (fract(x)*2-1);
    // double lin_y = (fract(y)*2-1);
    
    // color.rgb *= vec3((abs(lin_x)) * (abs(lin_y)));
    // color.rgb *= 1-distance(vec2(lin_x, lin_y), vec2(0));
    // double d = atan(float(lin_y) - sin(time)*0.25, float(lin_x) - cos(time)*0.25);
    // double r = degrees(atan(float(lin_y+cos(time)*0.5), float(lin_x+cos(time)*0.5)));
    // if ((r) > -135)
        // color.rgb = vec3((r+180)/360, 0, 0);
    // else
    //     color.rgb = vec3(0, 0, 0);
    color.a = 1;
}