#version 330 core
in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uRuneTexture;
uniform vec3 uRuneColor;
uniform float uTime;
uniform float uActivity;

void main() {
    vec4 tex = texture(uRuneTexture, vTexCoords);
    if (tex.a < 0.05) {
        discard;
    }

    // Gentle arcane pulse
    float pulse = 0.85 + 0.35 * sin(uTime * 3.0 + length(vTexCoords - vec2(0.5)) * 10.0);
    float glow = (0.5 + 0.8 * uActivity) * pulse;

    vec3 finalRuneColor = mix(tex.rgb, uRuneColor, 0.7) * glow;
    FragColor = vec4(finalRuneColor, tex.a * 0.9);
}
