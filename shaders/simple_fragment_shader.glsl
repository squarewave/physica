#version 330 core

uniform vec4 draw_color;

layout(location = 0) out vec4 color;
layout(location = 1) out int texture_id_out;

void main() {
    color = draw_color;
    texture_id_out = -1;
}