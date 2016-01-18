#version 330 core
 
in vec2 uv;
layout(location = 0) out vec4 color;
layout(location = 1) out int texture_id_out;
 
uniform vec4 tint;
uniform vec4 lighting;
uniform int texture_id_in;
uniform sampler2D texture_sampler;
 
void main() {
    vec4 texel = texture(texture_sampler, uv);
    if(texel.a < 0.5) {
        discard;
    }

    vec4 tinted = vec4(tint.a * tint.rgb + (1.0 - tint.a) * texel.rgb, texel.a);

    color = vec4(lighting.r * tinted.r,
                 lighting.g * tinted.g,
                 lighting.b * tinted.b,
                 tinted.a);

    texture_id_out = texture_id_in;
}