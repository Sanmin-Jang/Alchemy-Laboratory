#version 330 core
in vec3 vLocalPos;
in vec2 vTexCoords;

out vec4 FragColor;

uniform float uTime;
uniform vec3 uFlameTint;

void main() {
    float t = clamp(vLocalPos.y / 0.18, 0.0, 1.0);

    // Thermal flame gradient: blue-white core -> hot gold -> amber-orange tip
    vec3 coreColor = vec3(0.92, 0.96, 1.0);
    vec3 midColor  = vec3(1.0, 0.72, 0.15);
    vec3 tipColor  = vec3(0.95, 0.22, 0.04);

    vec3 col = (t < 0.35) ? mix(coreColor, midColor, t / 0.35)
                          : mix(midColor, tipColor, (t - 0.35) / 0.65);

    // Tint flames cool cyan during Glacial Mist effect, warm normally
    col *= uFlameTint;

    float alpha = smoothstep(1.0, 0.08, t) * 0.95;
    FragColor = vec4(col * 1.5, alpha);
}
