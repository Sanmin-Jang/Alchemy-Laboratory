#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define NUM_LIGHTS 4
uniform PointLight uLights[NUM_LIGHTS];

uniform vec3 uViewPos;
uniform vec3 uDiffuseColor;
uniform vec3 uSpecularColor;
uniform float uShininess;

uniform bool uUseTexture;
uniform sampler2D uTexture;
uniform vec3 uEmissive;

// Atmospheric dungeon fog uniforms
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uAmbientColor;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);

    vec4 baseTex = uUseTexture ? texture(uTexture, vTexCoords) : vec4(1.0);
    vec3 albedo = uDiffuseColor * baseTex.rgb;

    // Atmospheric dark dungeon ambient + emissive term
    vec3 ambient = uAmbientColor * albedo;
    vec3 lighting = ambient + uEmissive;

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        if (uLights[i].intensity <= 0.001) continue;

        vec3 lightDir = uLights[i].position - vFragPos;
        float dist = length(lightDir);
        lightDir = normalize(lightDir);

        // Blinn-Phong Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * albedo * uLights[i].color;

        // Blinn-Phong Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), max(uShininess, 1.0));
        vec3 specular = spec * uSpecularColor * uLights[i].color;

        // Quadratic distance attenuation
        float attenuation = 1.0 / (1.0 + 0.14 * dist + 0.07 * dist * dist);

        lighting += (diffuse + specular) * uLights[i].intensity * attenuation;
    }

    // Exponential squared atmospheric fog for dungeon depth
    float viewDist = length(uViewPos - vFragPos);
    float fogFactor = exp(-pow(viewDist * uFogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 finalColor = mix(uFogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, baseTex.a);
}
