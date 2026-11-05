#version 330 core
in vec3 vFragPos;
in vec3 vNormal;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 3
uniform PointLight uLights[NUM_LIGHTS];

uniform vec3 uViewPos;
uniform vec3 uGlassTint;
uniform float uGlassAlpha;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);

    // Fresnel reflectance factor: edges reflect much more light
    float NdotV = max(dot(norm, viewDir), 0.0);
    float fresnel = pow(1.0 - NdotV, 3.2);

    // Specular highlights from all lights
    vec3 specTotal = vec3(0.0);
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        if (uLights[i].intensity <= 0.0) continue;
        vec3 lDir = uLights[i].position - vFragPos;
        float dist = length(lDir);
        lDir = normalize(lDir);

        vec3 halfDir = normalize(lDir + viewDir);
        float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);

        float atten = 1.0 / (1.0 + 0.18 * dist + 0.10 * dist * dist);
        specTotal += spec * uLights[i].color * uLights[i].intensity * atten;
    }

    vec3 ambientGlass = uGlassTint * 0.15;
    vec3 reflection = vec3(0.95, 0.98, 1.0) * fresnel * 0.65;
    vec3 resultColor = ambientGlass + reflection + specTotal * 1.2;

    // Edge opacity increases with Fresnel angle
    float alpha = clamp(uGlassAlpha + fresnel * 0.55, 0.0, 0.95);

    FragColor = vec4(resultColor, alpha);
}
