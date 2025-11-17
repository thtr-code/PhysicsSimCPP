#version 330 core

in vec3 vColor;
out vec4 FragColor;

uniform vec3 baseColor;
uniform float isSun;

void main()
{
    // Make each point a circle
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(coord, coord);
    if (r2 > 1.0)
    discard;

    float r = sqrt(r2);
    float edge = smoothstep(1.0, 0.7, 1.0 - r);

    vec3 color = mix(vColor, baseColor, 0.6);

    if (isSun > 0.5) {
        // Soft radial glow
        float glow = pow(1.0 - clamp(r, 0.0, 1.0), 2.0);
        color += vec3(1.0, 0.9, 0.7) * glow;
    }

    FragColor = vec4(color, edge);
}
