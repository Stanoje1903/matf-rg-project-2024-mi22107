//#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
//#shader fragment
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;
uniform float bloomStrength;

void main() {
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb * bloomStrength;
    vec3 result = hdrColor + bloomColor;

    result = vec3(1.0) - exp(-result * exposure);
    FragColor = vec4(result, 1.0);
}
