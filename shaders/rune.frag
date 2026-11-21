#version 330 core
in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uRuneTexture;
uniform vec3 uRuneColor;
uniform float uTime;
uniform float uActivity;
uniform float uRotation;

void main() {
    // Rotate UVs around circle center (0.5, 0.5)
    vec2 centered = vTexCoords - vec2(0.5);
    float c = cos(uRotation);
    float s = sin(uRotation);
    vec2 rotUV = vec2(centered.x * c - centered.y * s,
                      centered.x * s + centered.y * c) + vec2(0.5);

    if (rotUV.x < 0.0 || rotUV.x > 1.0 || rotUV.y < 0.0 || rotUV.y > 1.0) {
        discard;
    }

    vec4 tex = texture(uRuneTexture, rotUV);
    if (tex.a < 0.05) {
        discard;
    }

    // Arcane radial pulse wave
    float r = length(centered);
    float pulse = 0.85 + 0.35 * sin(uTime * 3.5 - r * 14.0);
    float glow = (0.45 + 1.35 * uActivity) * pulse;

    vec3 finalRuneColor = mix(tex.rgb, uRuneColor, 0.80) * glow;
    FragColor = vec4(finalRuneColor, tex.a * clamp(0.65 + 0.35 * uActivity, 0.0, 1.0));
}
