#version 330 core

uniform mat4 projMatrix;
uniform mat4 modelMatrix;

uniform vec3 cellTypeColor;
uniform int type;

layout(location = 0) in vec3 position;
layout(location = 1) in float radius;

out float pass_radius;
out vec3 pass_color;

void main()
{
    pass_radius = radius;
    
    vec3 color = vec3(1.0, 1.0, 1.0);

    // Soma
    if (type == 1)
    {
        color = vec3(0.706, 0.22, 0.38);
    }
    // Axon
    else if (type == 2)
    {
        color = vec3(0.56, 0.56, 0.56);
    }
    // Basal dendrite
    else if (type == 3)
    {
        color = cellTypeColor * 0.8;
    }
    // Apical dendrite
    else if (type == 4)
    {
        color = cellTypeColor;
    }

    pass_color = color;

    gl_Position = projMatrix * modelMatrix * vec4(position, 1);
}
