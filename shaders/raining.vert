#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out float gradient;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);

    //textures coord
    TexCoords = vec2(position.x, position.y);

    //gradient efect
    gradient = 1.0 - abs(position.y);
}
