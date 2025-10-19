#version 410 core

in vec2 TexCoords;
in float gradient;

out vec4 fragColor;

uniform vec3 lightDir;
uniform float time;

void main() {
    //simulate specular highlight based on gradient
    float specular = pow(gradient, 3.0);

    //add a subtle flicker effect using sine waves
    float flicker = 0.8 + 0.2 * sin(TexCoords.y * 10.0 + time * 5.0);

    //create the main color of the raindrop
    vec3 color = vec3(0.4, 0.6, 1.0) * (0.7 + 0.3 * gradient) * flicker;

    //alpha transparency for tapering edges
    float alpha = 0.7 * (1.0 - abs(TexCoords.y));

    fragColor = vec4(color, alpha);
}
