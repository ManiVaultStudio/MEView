#version 330 core

vec3 vertices[4] = vec3[] (
    vec3(-1, -1, 0),
    vec3(1, -1, 0),
    vec3(-1, 1, 0),
    vec3(1, 1, 0)
);

vec2 texCoords[4] = vec2[] (
    vec2(0, 1),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 0)
);

uniform mat4 projMatrix;
uniform mat4 modelMatrix;

out vec3 pass_position;
out vec2 pass_texCoord;

void main() {
    vec3 position = vertices[gl_VertexID];

    pass_position = position;
    pass_texCoord = texCoords[gl_VertexID];

    gl_Position = projMatrix * modelMatrix * vec4(position, 1);
}
