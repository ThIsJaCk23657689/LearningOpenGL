#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
	float shininess;
	float intensity;
};

struct DirLight {
	vec3 direction;
	vec3 color;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;
	vec3 color;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 color;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
	float cutoff;
	float outerCutoff;
};

#define NR_POINT_LIGHT 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform float time;
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirlight;
uniform PointLight pointlights[NR_POINT_LIGHT];
uniform SpotLight spotlight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	// Phase One: Define direct light
	vec3 result = CalcDirLight(dirlight, norm, viewDir);

	// Phase Two: Define point lights
	for(int i = 0; i < NR_POINT_LIGHT; i++) {
		result += CalcPointLight(pointlights[i], norm, FragPos, viewDir);
	}

	// Phase Three: Define spotlight
	result += CalcSpotLight(spotlight, norm, FragPos, viewDir);

	// Calculate Emission
	vec3 emission = vec3(0.0);
	if (texture(material.specular, TexCoords).r == 0.0) {
		emission = texture(material.emission, TexCoords + vec2(0.0, time)).rgb;
		emission = emission * material.intensity * vec3((sin(time * 2) * 0.5 + 0.5) * 2, (sin(time / 5) * 0.5 + 0.5) / 5, (sin(time) * 0.5 + 0.5) * 3);
	}

	FragColor = vec4(result + emission, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	// Direct light Process

	// ambient
	vec3 ambient = light.ambient * texture(material.specular, TexCoords).rgb * light.color;

	// diffuse
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.specular, TexCoords).rgb * light.color;
	
	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb * light.color;
	
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	// Point light Process

	// ambient
	vec3 ambient = light.ambient * texture(material.specular, TexCoords).rgb * light.color;

	// diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.specular, TexCoords).rgb * light.color;

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb * light.color; 

	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	// Spotlight Process

	// ambient
	vec3 ambient = light.ambient * texture(material.specular, TexCoords).rgb * light.color;
	
	// diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.specular, TexCoords).rgb * light.color;

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb * light.color;
	
	// spotlight (soft edge)
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;

	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}