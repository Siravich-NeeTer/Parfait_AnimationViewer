#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform PushConsts 
{
	mat4 model;
} primitive;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    gl_Position = ubo.projection * ubo.view * primitive.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}