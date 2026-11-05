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

#define NUM_LIGHTS 3
uniform PointLight uLights[NUM_LIGHTS];

uniform vec3 uViewPos;
uniform vec3 uDiffuseColor;
uniform vec3 uSpecularColor;
uniform float uShininess;
uniform float uRoughness;

uniform bool uUseTexture;
uniform sampler2D uTexture;
uniform vec3 uEmissive;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);

    vec4 baseTex = uUseTexture ? texture(uTexture, vTexCoords) : vec4(1.0);
    vec3 albedo = uDiffuseColor * baseTex.rgb;

    // Atmospheric dark room ambient
    vec3 ambient = 0.06 * albedo;
    vec3 lighting = ambient + uEmissive;

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        if (uLights[i].intensity <= 0.0) continue;

        vec3 lightDir = uLights[i].position - vFragPos;
        float dist = length(lightDir);
        lightDir = normalize(lightDir);

        // Blinn-Phong Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * albedo * uLights[i].color;

        // Blinn-Phong Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), uShininess);
        vec3 specular = spec * uSpecularColor * uLights[i].color;

        // Realistic quadratic distance attenuation
        float attenuation = 1.0 / (1.0 + 0.18 * dist + 0.10 * dist * dist);

        lighting += (diffuse + specular) * uLights[i].intensity * attenuation;
    }

    FragColor = vec4(lighting, baseTex.a);
}
