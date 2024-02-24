#version 450

// vertex input layout
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// vertex output layout
layout(location = 0) out vec3 fragColor;

// constant
layout(push_constant) uniform Push
{
    mat4 transform;
    vec3 color;
} push;


void main()
{
    // position is a column vector
    gl_Position = push.transform * vec4(position, 1.0f);
    fragColor = color;
}