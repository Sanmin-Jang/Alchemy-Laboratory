#version 330 core
in vec3 vLocalPos;
in vec3 vFragPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform vec4 uLiquidColor;
uniform float uFillLevel;
uniform vec3 uSloshNormal;
uniform float uFlaskMaxHeight;
uniform float uTime;
uniform float uWaveAmp;
uniform float uWavePhase;
uniform bool uIsTopCap;

float getInnerFlaskRadius(float y) {
    float r = 0.05;
    if (y <= 0.02) {
        r = 0.22 - (0.22 - 0.209) * (y / 0.02);
    } else if (y <= 0.31) {
        float t = (y - 0.02) / (0.31 - 0.02);
        r = 0.209 + (0.085 - 0.209) * t;
    } else if (y <= 0.34) {
        float t = (y - 0.31) / 0.03;
        r = 0.085 + (0.05 - 0.085) * t;
    } else {
        r = 0.05;
    }
    return r * 0.96;
}

void main() {
    vec3 nSlosh = normalize(uSloshNormal);
    float hNominal = uFlaskMaxHeight * uFillLevel;

    // Sloshing surface plane: nSlosh.x * x + nSlosh.y * (y - hNominal) + nSlosh.z * z = 0
    float surfaceY = hNominal - (nSlosh.x * vLocalPos.x + nSlosh.z * vLocalPos.z) / max(0.18, nSlosh.y);

    // Dynamic wave ripples excited by movement
    float radius = length(vLocalPos.xz);
    float wave = sin(radius * 26.0 - uWavePhase) * uWaveAmp * max(0.0, 1.0 - radius * 2.2);
    float limitY = surfaceY + wave;

    if (!uIsTopCap) {
        // Discard any fragment of the liquid volume above the dynamic surface
        if (vLocalPos.y > limitY) {
            discard;
        }
    } else {
        // For the top cap, clip to the inner flask boundary at this height
        float maxR = getInnerFlaskRadius(vLocalPos.y);
        if (radius > maxR) {
            discard;
        }
    }

    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 norm = normalize(vNormal);

    // If near the top cutoff, blend normal toward the slosh plane normal
    float distToSurface = abs(limitY - vLocalPos.y);
    if (!uIsTopCap && distToSurface < 0.035) {
        norm = normalize(mix(norm, nSlosh, 0.75));
    }

    // Depth-based color absorption (richer base inside, radiant near top surface)
    float depth = clamp(vLocalPos.y / max(0.01, hNominal), 0.0, 1.0);
    if (uIsTopCap) {
        depth = 1.0;
    }
    vec3 deepShade = uLiquidColor.rgb * 0.45;
    vec3 brightShade = uLiquidColor.rgb * 1.15;
    vec3 baseColor = mix(deepShade, brightShade, depth);

    // Glowing meniscus rim at fluid-glass contact
    float meniscus = 1.0 - smoothstep(0.0, 0.024, distToSurface);
    if (uIsTopCap) {
        float maxR = getInnerFlaskRadius(vLocalPos.y);
        float edgeDist = maxR - radius;
        meniscus = 1.0 - smoothstep(0.0, 0.022, edgeDist);
    }
    vec3 meniscusHighlight = mix(uLiquidColor.rgb, vec3(1.0), 0.45) * meniscus * 0.85;

    // Rising micro-bubbles (magical elixir effervescence)
    float b1 = sin(vLocalPos.x * 60.0 + sin(vLocalPos.y * 35.0));
    float b2 = cos(vLocalPos.z * 60.0);
    float b3 = sin(vLocalPos.y * 45.0 - uTime * 3.8);
    float sparkle = b1 * b2 * b3;
    vec3 bubbleSparkle = vec3(0.0);
    if (sparkle > 0.82) {
        bubbleSparkle = mix(uLiquidColor.rgb, vec3(1.0), 0.6) * (sparkle - 0.82) * 5.0;
    }

    // Specular glint for 3D liquid curvature and surface sheen
    vec3 keyDir = normalize(vec3(0.4, 0.9, 0.7));
    vec3 halfDir = normalize(keyDir + viewDir);
    float specPower = uIsTopCap ? 48.0 : 32.0;
    float spec = pow(max(dot(norm, halfDir), 0.0), specPower);
    vec3 specularGlint = vec3(0.95, 1.0, 0.95) * spec * (uIsTopCap ? 0.75 : 0.45);

    // Ambient + subtle directional curvature shading
    float diff = max(dot(norm, keyDir), 0.0) * 0.35 + 0.65;

    vec3 finalColor = baseColor * diff + meniscusHighlight + bubbleSparkle + specularGlint;
    FragColor = vec4(finalColor, uLiquidColor.a);
}
