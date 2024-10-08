#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform PushConsts 
{
	mat4 model;
    int numBones;
} primitive;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = ubo.projection * ubo.view * primitive.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}