#version 330 core
in vec3 vLocalPos;
in vec3 vFragPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform bool uIsBackPass;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 2
uniform PointLight uLights[NUM_LIGHTS];

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    if (uIsBackPass) {
        // Inner glass surface pass: gentle translucent tint
        vec3 innerTint = vec3(0.72, 0.90, 0.88);
        FragColor = vec4(innerTint, 0.14);
        return;
    }

    // Front glass surface pass
    // Fresnel reflectance equation: R = (1 - N.V)^3
    float NdotV = max(dot(norm, viewDir), 0.0);
    float fresnel = pow(1.0 - NdotV, 3.2);

    // Laboratory reflections and specular glints
    vec3 lightDir1 = normalize(vec3(0.5, 1.0, 0.8));
    vec3 half1 = normalize(lightDir1 + viewDir);
    float spec1 = pow(max(dot(norm, half1), 0.0), 64.0);

    vec3 lightDir2 = normalize(vec3(-0.6, 0.8, -0.4));
    vec3 half2 = normalize(lightDir2 + viewDir);
    float spec2 = pow(max(dot(norm, half2), 0.0), 32.0);

    // Dynamic candle glints dancing on the curved glass body
    vec3 candleGlints = vec3(0.0);
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        if (uLights[i].intensity <= 0.0) continue;
        vec3 lDir = uLights[i].position - vFragPos;
        float dist = length(lDir);
        lDir = normalize(lDir);
        vec3 hDir = normalize(lDir + viewDir);
        float s = pow(max(dot(norm, hDir), 0.0), 48.0);
        float atten = 1.0 / (1.0 + 0.30 * dist + 0.20 * dist * dist);
        candleGlints += s * uLights[i].color * (uLights[i].intensity * atten);
    }

    vec3 glassColor = vec3(0.85, 0.96, 1.0) * fresnel * 0.85 
                    + vec3(1.0) * (spec1 * 0.7 + spec2 * 0.3) 
                    + candleGlints * 1.6;
    float alpha = clamp(0.16 + fresnel * 0.65 + spec1 * 0.4 + length(candleGlints) * 0.4, 0.0, 0.95);

    FragColor = vec4(glassColor, alpha);
}
