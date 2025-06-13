#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec3 cellTypeColor;

layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in int type;

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
        color = vec3(0.8, 0.8, 0.8);
    }
    // Basal dendrite
    else if (type == 3)
    {
        color = vec3(0.5, 0.5, 0.5);
    }
    // Apical dendrite
    else if (type == 4)
    {
        color = cellTypeColor;
    }

    pass_color = color;

    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1);
}
