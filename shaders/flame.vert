#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vLocalPos;
out vec2 vTexCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;

void main() {
    vec3 animatedPos = aPos;
    // Teardrop flame wobbles more towards the tip
    float heightFactor = clamp(aPos.y / 0.18, 0.0, 1.0);
    animatedPos.x += sin(uTime * 14.0 + aPos.y * 25.0) * 0.015 * heightFactor;
    animatedPos.z += cos(uTime * 11.0 + aPos.y * 20.0) * 0.012 * heightFactor;
    animatedPos.y *= 1.0 + sin(uTime * 18.0) * 0.06;

    vLocalPos = animatedPos;
    vTexCoords = aTexCoords;

    gl_Position = uProjection * uView * uModel * vec4(animatedPos, 1.0);
}
