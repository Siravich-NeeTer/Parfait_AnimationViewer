#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inID;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform PushConsts 
{
	mat4 model;
} primitive;

layout(location = 0) out uint outID;
layout(location = 1) out float zDepth;

void main() 
{
    gl_Position = ubo.projection * ubo.view * primitive.model * vec4(inPosition, 1.0);
    zDepth = gl_Position.z / gl_Position.w;
    outID = inID;
}