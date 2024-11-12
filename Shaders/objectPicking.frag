#version 460

layout(location = 0) flat in uint ID;
layout(location = 1) in float zDepth;
 
layout(set = 1, binding = 0) buffer ShaderStorageBufferObject
{
    uint Selected_ID;
    float minDepth;
}ssbo;
 
layout(location = 0) out float outColor;
 
void main()
{
    if(zDepth > ssbo.minDepth)
        return;

    ssbo.Selected_ID = ID;
    ssbo.minDepth = zDepth;
   
    //only needed for debugging to draw to color attachment
    outColor = ssbo.Selected_ID;
}