#version 330 core

in vec2 vertex_uv;
in vec3 vertex_modelspace;
out vec2 uv;

uniform mat4 texture_transform;
uniform mat3 uv_transform;

void main() {
    gl_Position = texture_transform * vec4(vertex_modelspace, 1.0);
    uv = (uv_transform * vec3(vertex_uv, 1.0f)).xy;
}