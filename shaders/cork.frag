#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 FragColor;

uniform vec3 uCameraPos;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.4, 0.9, 0.6));
    float diff = max(dot(norm, lightDir), 0.0) * 0.55 + 0.45;

    // Procedural cork texture grain
    vec3 corkColor = vec3(0.68, 0.48, 0.30);
    float grain = sin(vFragPos.y * 120.0 + sin(vFragPos.x * 80.0)) * 0.08;
    corkColor += vec3(grain);

    FragColor = vec4(corkColor * diff, 1.0);
}
