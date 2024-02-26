#version 450

// vertex input layout
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;


// vertex output layout
layout(location = 0) out vec3 fragColor;

// constant
layout(push_constant) uniform Push
{
    mat4 transform; // proj * view * model
    mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0f, -3.0f, -1.0f));
const float AMBIENT = 0.02f;

void main()
{
    // position is a column vector
    gl_Position = push.transform * vec4(position, 1.0f);
    
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0.0f);

    fragColor = lightIntensity * color;
}