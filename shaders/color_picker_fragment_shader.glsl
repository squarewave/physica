#version 330 core

uniform vec3 hsv;
uniform vec2 min_p;
uniform vec2 max_p;
out vec4 color;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float saturation;
    float value;
    float hue;

    vec2 normalized_p =
        vec2((gl_FragCoord.x - min_p.x) / (max_p.x - min_p.x),
             (gl_FragCoord.y - min_p.y) / (max_p.y - min_p.y));
    if (normalized_p.x < 0.9) {
        vec2 delta = normalized_p - hsv.yz;

        saturation = normalized_p.x / 0.9;
        value = normalized_p.y;
        hue = hsv.x;

        if (abs(delta.x) <= 0.003 || abs(delta.y) <= 0.003) {
            saturation = 0.0;
            value = 0.0;
            hue = 0.0;
        }
    } else {
        if (abs(normalized_p.y - hsv.x) > 0.003) {
            saturation = 1.0;
            value = 1.0;
            hue = normalized_p.y;
        } else {
            saturation = 0.0;
            value = 0.0;
            hue = 0.0;
        }
    }

    color = vec4(hsv2rgb(vec3(hue, saturation, value)), 1.0);
}