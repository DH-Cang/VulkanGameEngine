#version 450

// vertex input layout
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// vertex output layout
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTexCoord;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

// set0: per frame constant
layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor; // w is intensity
    PointLight PointLights[10];
    int numLights;
} ubo;

// set1: per object constant
layout(set = 1, binding = 0) uniform PerObjectUbo
{
    mat4 modelMatrix; // model
    mat4 normalMatrix;
} perObjectUbo;

void main()
{
    vec4 positionWorld = perObjectUbo.modelMatrix * vec4(position, 1.0f); // position is a column vector
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * positionWorld;
    fragNormalWorld = normalize(mat3(perObjectUbo.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    fragTexCoord = uv;
}