#version 330 core
layout (location = 0) in vec3 aQuadPos;
layout (location = 2) in vec2 aTexCoords;

// Per-instance attributes (divisor = 1)
layout (location = 3) in vec4 aInstancePosSize; // xyz = world pos, w = size
layout (location = 4) in vec4 aInstanceColor;   // rgba

out vec2 vTexCoords;
out vec4 vColor;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    // Extract camera right & up vectors from view matrix for spherical billboarding
    vec3 camRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 camUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);

    vec3 worldPos = aInstancePosSize.xyz
                  + camRight * (aQuadPos.x * aInstancePosSize.w)
                  + camUp    * (aQuadPos.y * aInstancePosSize.w);

    vTexCoords = aTexCoords;
    vColor = aInstanceColor;

    gl_Position = uProjection * uView * vec4(worldPos, 1.0);
}
