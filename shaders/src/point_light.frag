#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

// uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor; // w is intensity
} ubo;

void main()
{
    float disSquare = dot(fragOffset, fragOffset);
    if(disSquare >= 1.0){
        discard;
    }
    outColor = vec4(ubo.lightColor.xyz, 1.0f);
}