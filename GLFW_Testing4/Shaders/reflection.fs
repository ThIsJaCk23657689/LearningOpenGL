#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture1;

void main() {    
    vec3 viewDir = normalize(Position - cameraPos);
    vec3 reflectDir = reflect(viewDir, normalize(Normal));
    FragColor = vec4(texture(skybox, reflectDir).rgb, 1.0) * texture(texture1, TexCoords);
}