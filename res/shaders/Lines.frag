#version 330 core

in vec4 pass_color;

out vec4 fragColor;

void main()
{
    fragColor = vec4(pass_color.rgb, 1);
}
