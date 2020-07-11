#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;

layout(binding = 1) uniform sampler2D rgb_texture;

layout(location = 0) out vec4 pixel;

void main()
{
    pixel = texture(rgb_texture, uv);
}