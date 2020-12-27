#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;
layout(location = 2) in float hue_shift;

layout(binding = 1) uniform sampler2D rgb_texture;

layout(location = 0) out vec4 pixel;

const mat3 rgb2yiq = mat3(0.299, 0.587, 0.114, 0.595716, -0.274453, -0.321263, 0.211456, -0.522591, 0.311135);
const mat3 yiq2rgb = mat3(1.0, 0.9563, 0.6210, 1.0, -0.2721, -0.6474, 1.0, -1.1070, 1.7046);

void main()
{
    vec3 ycolor = rgb2yiq * texture(rgb_texture, uv).rgb;
    float original_hue = atan(ycolor.b, ycolor.g);
    float final_hue = original_hue + hue_shift;
    float chroma = sqrt(ycolor.b * ycolor.b + ycolor.g * ycolor.g);

    vec3 yfinalcolor = vec3(ycolor.r, chroma * cos(final_hue), chroma * sin(final_hue));

    pixel = vec4(yiq2rgb * yfinalcolor, 1.0);
}