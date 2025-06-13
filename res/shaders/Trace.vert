#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = projMatrix * viewMatrix * vec4(position, 1);
}
