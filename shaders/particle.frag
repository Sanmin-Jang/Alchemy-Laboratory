#version 330 core
in vec2 vTexCoords;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uParticleTex;

void main() {
    vec4 mask = texture(uParticleTex, vTexCoords);
    float alpha = mask.a * vColor.a;
    if (alpha < 0.01) {
        discard;
    }
    FragColor = vec4(vColor.rgb * mask.rgb, alpha);
}
