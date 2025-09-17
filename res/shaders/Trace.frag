#version 330 core

out vec4 fragColor;

//uniform vec3 cellTypeColor; // Not using it because e.g. yellow is problematic on white background
uniform vec3 lineColor;
uniform float alpha;

void main()
{
    fragColor = vec4(lineColor, alpha);
}
