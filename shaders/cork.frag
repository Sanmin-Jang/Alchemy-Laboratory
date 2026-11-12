#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 FragColor;

uniform vec3 uCameraPos;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 2
uniform PointLight uLights[NUM_LIGHTS];

void main() {
    vec3 norm = normalize(vNormal);

    // Procedural cork texture grain
    vec3 corkColor = vec3(0.68, 0.48, 0.30);
    float grain = sin(vFragPos.y * 120.0 + sin(vFragPos.x * 80.0)) * 0.08;
    corkColor += vec3(grain);

    // Ambient + candle diffuse
    vec3 totalLight = corkColor * 0.18;
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        if (uLights[i].intensity <= 0.0) continue;
        vec3 lDir = uLights[i].position - vFragPos;
        float dist = length(lDir);
        lDir = normalize(lDir);
        float diff = max(dot(norm, lDir), 0.0);
        float atten = 1.0 / (1.0 + 0.30 * dist + 0.20 * dist * dist);
        totalLight += diff * corkColor * uLights[i].color * (uLights[i].intensity * atten);
    }

    FragColor = vec4(totalLight, 1.0);
}
