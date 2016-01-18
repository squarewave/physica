#version 330 core

uniform vec2 viewport;
uniform vec2 gradient_start;
uniform vec2 gradient_end;
uniform vec4 start_color;
uniform vec4 end_color;
    
layout(location = 0) out vec4 color;
layout(location = 1) out int texture_id_out;

void main() {
    vec2 coord = vec2(2.0 * gl_FragCoord.x / viewport.x - 1.0,
                      2.0 * gl_FragCoord.y / viewport.y - 1.0);
    vec2 gradient_delta = gradient_end - gradient_start;
    float gradient_magnitude = sqrt(gradient_delta.x * gradient_delta.x +
                                   gradient_delta.y * gradient_delta.y);
    vec2 gradient_direction = gradient_delta / gradient_magnitude;
    vec2 delta = coord - gradient_start;
    float delta_magnitude = dot(delta, gradient_direction);
    float ratio = clamp(delta_magnitude / gradient_magnitude, 0.0, 1.0);

    color = ratio * end_color + (1.0 - ratio) * start_color;
    texture_id_out = -1;
    // color = vec4(ratio,ratio,ratio,1.0f);
}