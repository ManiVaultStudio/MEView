#version 330 core

in vec3 pass_position;
in vec2 pass_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = vec4(0, 0, 1, 0.5);
}
