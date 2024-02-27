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
    vec3 directionToLight;
} ubo;

// constant
layout(push_constant) uniform Push
{
    mat4 modelMatrix; // model
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.02f;

void main()
{
    // position is a column vector
    gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0f);
    
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0.0f);

    fragColor = lightIntensity * color;
}