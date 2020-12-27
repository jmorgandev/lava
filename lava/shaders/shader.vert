#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 transform;
    mat4 view;
    mat4 proj;
    uniform float hue_shift;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec2 uv_out;
layout(location = 2) out float hue_shift_out;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.transform * vec4(position, 1.0);
    color_out = color;
    uv_out = texcoord;
    hue_shift_out = ubo.hue_shift;
}