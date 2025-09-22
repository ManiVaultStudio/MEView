#version 330 core

in vec2 pass_texCoord;

out vec4 fragColor;

void main()
{
    // Circle rendering
    vec2 uv = pass_texCoord;

    // Distance from center
    float dist = distance(uv, vec2(0.5));

    // Smooth edge
    float alpha = 1.0 - smoothstep(0.5 - 0.05, 0.5 + 0.05, dist);

    fragColor = vec4(0.9, 0.0, 0.0, alpha);
}
