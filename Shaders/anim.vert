#version 460

const int MAX_BONE_INFLUENCE = 4;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in ivec4 inBoneIDs;
layout(location = 5) in vec4 inWeights;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 projection;
} ubo;
layout(std140, set = 2, binding = 0) readonly buffer BoneTransform
{
    mat4 bone[];
} boneTransform;

layout(push_constant) uniform PushConsts 
{
	mat4 model;
    int numBones;
    int boneOffset;
    int isAnimation;
} primitive;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

void main() 
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    int cnt = 0;
    if(primitive.isAnimation != 0)
    {
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if(inBoneIDs[i] == -1)
                continue;
            if(inBoneIDs[i] >= primitive.numBones)
            {
                totalPosition += vec4(inPosition, 1.0f);
                break;
            }

            vec4 localPosition = boneTransform.bone[primitive.boneOffset + inBoneIDs[i]] * vec4(inPosition, 1.0f);
            totalPosition += localPosition * inWeights[i];
            totalNormal += mat3(boneTransform.bone[primitive.boneOffset + inBoneIDs[i]]) * inNormal;
            cnt++;
        }
    }
    else
    {
        totalNormal = inNormal;
        cnt = 1;
        totalPosition = vec4(inPosition, 1.0f);
    }

    gl_Position = ubo.projection * ubo.view * primitive.model * totalPosition;
    fragColor = inColor;
    fragNormal = normalize((primitive.model * vec4(totalNormal / cnt, 0.0f)).xyz);
    fragTexCoord = inTexCoord;
}