#version 330 core

in vec4 vertex_draw_color;
uniform vec4 lighting;
out vec4 color;

void main() {
    color = vec4(lighting.r * vertex_draw_color.r,
                 lighting.g * vertex_draw_color.g,
                 lighting.b * vertex_draw_color.b,
                 vertex_draw_color.a);
}