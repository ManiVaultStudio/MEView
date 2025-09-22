#version 330 core

uniform mat4 projMatrix;

//uniform float somaRadius;
uniform vec3 somaPosition;

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

out vec2 pass_texCoord;

void main()
{
    vec3 vertexPos = position.xyz * 0.01 + somaPosition;
    pass_texCoord = texCoord;
    gl_Position = projMatrix * vec4(vertexPos, 1);
}
