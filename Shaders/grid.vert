// Resources (Marie): https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
// Modified by: NeeTer

#version 460

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 projection;
} ubo;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[] (
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

layout(location = 0) out float near;
layout(location = 1) out float far;
layout(location = 2) out vec3 nearPoint;
layout(location = 3) out vec3 farPoint;
layout(location = 4) out mat4 fragView;
layout(location = 8) out mat4 fragProj;
layout(location = 12) out vec3 cameraPosition;

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) 
{
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// normal vertice projection
void main() 
{
    vec3 p = gridPlane[gl_VertexIndex].xyz;

    near = 0.01f;
    far = 10.0f;

    // unprojecting on the near plane
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, ubo.view, ubo.projection).xyz; 
    // unprojecting on the far plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, ubo.view, ubo.projection).xyz; 
    
    fragView = ubo.view;
    fragProj = ubo.projection;
    
    mat4 viewInv = inverse(ubo.view);
    cameraPosition = -viewInv[3].xyz; // Extract camera position

    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
}