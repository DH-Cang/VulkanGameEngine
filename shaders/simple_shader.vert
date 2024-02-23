#version 450

// vertex input layout
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// vertex output layout
//layout(location = 0) out vec3 fragColor;

// constant
layout(push_constant) uniform Push
{
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;


void main()
{
    // position is a column vector
    gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
}