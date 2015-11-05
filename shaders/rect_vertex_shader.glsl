#version 150 core

in vec3 vertex_modelspace;

uniform mat4 rect_transform;

void main() {
    vec4 v = vec4(vertex_modelspace, 1.0);
    gl_Position = rect_transform * v;
}