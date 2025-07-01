#version 330 core

out vec4 fragColor;

uniform vec3 cellTypeColor; // Not using it because e.g. yellow is problematic on white background

void main()
{
    fragColor = vec4(0.2, 0.4, 0.839, 1);
}
