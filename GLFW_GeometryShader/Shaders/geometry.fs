#version 330 core
out vec4 FragColor;

in vec3 fColor;

uniform sampler2D texture1;

void main() {    
    FragColor = vec4(fColor, 1.0);
}