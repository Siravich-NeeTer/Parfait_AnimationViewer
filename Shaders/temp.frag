#version 450

const int MAX_TEXTURE_SAMPLER = 20;
layout(binding = 1) uniform sampler2D texSampler[MAX_TEXTURE_SAMPLER];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int fragTexIndex;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(texSampler[fragTexIndex], fragTexCoord);
}