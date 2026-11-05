#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vLocalPos;
out vec3 vFragPos;
out vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

uniform bool uIsTopCap;
uniform float uFillLevel;
uniform vec3 uSloshNormal;
uniform float uFlaskMaxHeight;
uniform float uWaveAmp;
uniform float uWavePhase;

void main() {
    vec3 pos = aPos;
    vec3 norm = aNormal;

    if (uIsTopCap) {
        vec3 nSlosh = normalize(uSloshNormal);
        float hNominal = uFlaskMaxHeight * uFillLevel;
        float surfaceY = hNominal - (nSlosh.x * aPos.x + nSlosh.z * aPos.z) / max(0.18, nSlosh.y);

        float radius = length(aPos.xz);
        float wave = sin(radius * 26.0 - uWavePhase) * uWaveAmp * max(0.0, 1.0 - radius * 2.2);
        pos = vec3(aPos.x, surfaceY + wave, aPos.z);
        norm = nSlosh;
    }

    vLocalPos = pos;
    vec4 worldPos = uModel * vec4(pos, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = normalize(uNormalMatrix * norm);
    gl_Position = uProjection * uView * worldPos;
}
