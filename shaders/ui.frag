#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform vec4 uColor;
uniform bool uUseTexture;
uniform sampler2D uFontTex;

void main() {
    if (uUseTexture) {
        vec4 tex = texture(uFontTex, vUV);
        if (tex.a < 0.1) discard;
        FragColor = vec4(uColor.rgb * tex.rgb, uColor.a * tex.a);
    } else {
        FragColor = uColor;
    }
}
