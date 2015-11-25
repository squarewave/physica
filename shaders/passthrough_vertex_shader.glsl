#version 330 core

in vec3 vertex_modelspace;

void main() {
    gl_Position = vec4(vertex_modelspace, 1.0);
}