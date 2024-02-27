#version 450

// vertex input layout
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;


// vertex output layout
layout(location = 0) out vec3 fragColor;

// uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionViewMatrix;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor; // w is intensity
} ubo;

// constant
layout(push_constant) uniform Push
{
    mat4 modelMatrix; // model
    mat4 normalMatrix;
} push;

void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0f); // position is a column vector
    
    gl_Position = ubo.projectionViewMatrix * positionWorld;
    
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    vec3 directionToLight = ubo.lightPosition - positionWorld.xyz;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

    fragColor = (diffuseLight + ambientLight) * color;
}