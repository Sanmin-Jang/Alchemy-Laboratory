#version 330 core
in vec3 vLocalPos;
in vec3 vFragPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform bool uIsBackPass;

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

    vec3 glassColor = vec3(0.85, 0.96, 1.0) * fresnel * 0.85 + vec3(1.0) * (spec1 * 1.1 + spec2 * 0.4);
    float alpha = clamp(0.16 + fresnel * 0.65 + spec1 * 0.6, 0.0, 0.95);

    FragColor = vec4(glassColor, alpha);
}
