#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture1;

void main() {    
    // reflect
    //vec3 viewDir = normalize(Position - cameraPos);
    //vec3 reflectDir = reflect(viewDir, normalize(Normal));
    //FragColor = vec4(texture(skybox, reflectDir).rgb, 1.0);
       
    // refract
    float ratio = 1.00 / 1.33;
    vec3 viewDir = normalize(Position - cameraPos);
    vec3 refractDir = refract(viewDir, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, refractDir).rgb, 1.0);
}