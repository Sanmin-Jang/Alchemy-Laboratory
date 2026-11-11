#version 330 core
in vec2 vTexCoords;
out vec4 FragColor;

uniform vec3 uGlowColor;
uniform float uIntensity;

void main() {
    vec2 p = vTexCoords - vec2(0.5);
    float dist = length(p) * 2.0;
    if (dist > 1.0) discard;

    // Smooth radial exponential falloff
    float glow = exp(-dist * dist * 4.0) * (1.0 - smoothstep(0.4, 1.0, dist));
    FragColor = vec4(uGlowColor * glow * uIntensity, glow * uIntensity);
}
