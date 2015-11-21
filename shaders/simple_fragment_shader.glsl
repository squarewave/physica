#version 330 core

uniform vec4 draw_color;
out vec4 color;

void main() {
    color = draw_color;
}