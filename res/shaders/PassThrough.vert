#version 330 core

uniform mat4 projMatrix;
uniform mat4 modelMatrix;

uniform vec3 cellTypeColor;
uniform int type;

layout(location = 0) in vec3 position;
layout(location = 1) in float radius;

out float pass_radius;
out vec4 pass_color;

void main()
{
    pass_radius = radius;
    
    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

    // Soma
    if (type == 1)
    {
        color = vec3(0.706, 0.22, 0.38);
        color = vec4(0.9, 0.0, 0.0, 1.0);
    }
    // Axon
    else if (type == 2)
    {
        color = vec4(0.56, 0.56, 0.56, 0.1);
    }
    // Basal dendrite
    else if (type == 3)
    {
        color = vec4(cellTypeColor * 0.45, 1.0);
    }
    // Apical dendrite
    else if (type == 4)
    {
        color = vec4(cellTypeColor, 1.0);
    }

    pass_color = color;

    gl_Position = projMatrix * modelMatrix * vec4(position, 1);
}
