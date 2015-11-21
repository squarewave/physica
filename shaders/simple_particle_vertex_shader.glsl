#version 330 core

in vec3 modelspace;
in vec4 center;
in vec2 scaling;
in vec4 draw_color;

uniform mat4 view_transform;
out vec4 vertex_draw_color;

void main() {
    vec3 scaled = vec3(modelspace.x * scaling.x, modelspace.y * scaling.y, modelspace.z);
    vec4 v = vec4(scaled + center.xyz, center.w);
    gl_Position = view_transform * v;
    gl_Position.w = 1.0f;
    vertex_draw_color = draw_color;
}