#version 450

// frag input layout
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

// output
layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

// uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLightColor; // w is intensity
    PointLight PointLights[10];
    int numLights;
} ubo;

// constant
layout(push_constant) uniform Push
{
    mat4 modelMatrix; // model
    mat4 normalMatrix;
} push;

void main()
{
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for(int i=0; i < ubo.numLights; i++)
    {
        PointLight light = ubo.PointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        float cosAngleIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;
    }

    outColor = vec4(diffuseLight * fragColor, 1.0f);
}