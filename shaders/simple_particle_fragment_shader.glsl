#version 330 core

in vec4 vertex_draw_color;
out vec4 color;

void main() {
    color = vertex_draw_color;
}