#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

// set1 per object
layout(set = 1, binding = 0) uniform PerObjectUbo
{
    vec4 position;
    vec4 color;
    float radius;
} perObjectUbo;

const float M_PI = 3.1415926538;

void main()
{
    float disSquare = dot(fragOffset, fragOffset);
    if(disSquare >= 1.0){
        discard;
    }
    outColor = vec4(perObjectUbo.color.xyz, 0.5f * (cos(sqrt(disSquare) * M_PI) + 1.0f));
}