#version 330 core
in vec3 vLocalPos;
in vec2 vTexCoords;

out vec4 FragColor;

uniform float uTime;
uniform vec3 uFlameColor;

void main() {
    float t = clamp(vLocalPos.y / 0.18, 0.0, 1.0);

    // Color gradient: Inner core -> Hot gold -> Red/orange tip
    vec3 coreColor = vec3(0.9, 0.95, 1.0);
    vec3 midColor  = vec3(1.0, 0.72, 0.15);
    vec3 tipColor  = vec3(0.95, 0.22, 0.04);

    vec3 col = (t < 0.35) ? mix(coreColor, midColor, t / 0.35) : mix(midColor, tipColor, (t - 0.35) / 0.65);

    // Soft outer falloff
    float alpha = smoothstep(1.0, 0.1, t) * 0.95;

    FragColor = vec4(col * 1.5, alpha);
}
