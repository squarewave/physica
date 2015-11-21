#version 330 core
 
in vec2 uv;
out vec4 color;
 
uniform vec4 tint;
uniform sampler2D texture_sampler;
 
void main() {
    vec4 texel = texture(texture_sampler, uv);
    if(texel.a < 0.5) {
        discard;
    }

    color = vec4(tint.a * tint.rgb + (1.0 - tint.a) * texel.rgb, texel.a);
}