#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 pixel;

void main()
{
    pixel = vec4(uv, 0.0, 1.0);
}