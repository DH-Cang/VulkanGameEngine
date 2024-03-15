#version 450

// frag input layout
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTexCoord;

// output
layout (location = 0) out vec4 outColor;

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

// set2: per material constant
layout(set = 2, binding = 0) uniform MaterialUbo
{
    vec4 final_ambient; // ignore w
    float blinnFactor;
} ubo2;
layout(set = 2, binding = 1) uniform sampler2D texSampler2;

void main()
{
    vec3 specularLight = vec3(0.0f);
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz; // last column
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    for(int i=0; i < ubo.numLights; i++)
    {
        PointLight light = ubo.PointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);

        directionToLight = normalize(directionToLight);
        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;

        // specular
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 32.0f); // higher values -> sharper highlight
        specularLight += intensity * blinnTerm;
    }

    vec3 temp_albedo = pow(texture(texSampler2, fragTexCoord).xyz, vec3(1.0f / 2.2f));
    outColor = vec4(diffuseLight * temp_albedo + specularLight, 1.0f);
}