#version 330 core
out vec4 FragColor;

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;

	sampler2D diffuse_texture;
	sampler2D specular_texture;
	sampler2D emission_texture;

	bool enableColorTexture;
    bool enableSpecularTexture;
	bool enableEmission;
    bool enableEmissionTexture;
};

struct Light {
	vec3 position;
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutoff;
	float outerCutoff;

	bool enable;
	int caster;
};

#define NUM_LIGHTS 6

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;
uniform bool useBlinnPhong;
uniform bool useLighting;
uniform bool useDiffuseTexture;
uniform bool useSpecularTexture;
uniform bool useEmission;
uniform bool useGamma;
uniform float GammaValue;

uniform Material material;
uniform Light lights[NUM_LIGHTS];

vec3 CalcLight(Light light, vec3 normal, vec3 viewDir) {

	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	vec3 lightDir = vec3(0.0);
	if (light.caster == 0) {
		// Direction Light
		lightDir = normalize(-light.direction);
	} else {
		lightDir = normalize(light.position - fs_in.FragPos);
	}

	float diff = max(dot(normal, lightDir), 0.0);

	float spec = 0.0;
	if (useBlinnPhong) {
		vec3 halfway = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfway), 0.0), material.shininess);
	} else {
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	}

	if (useDiffuseTexture && material.enableColorTexture) {
		ambient = light.ambient * texture(material.diffuse_texture, fs_in.TexCoords).rgb;
		diffuse = light.diffuse * diff * texture(material.diffuse_texture, fs_in.TexCoords).rgb;
		if (useSpecularTexture && material.enableSpecularTexture) {
			specular = light.specular * spec * texture(material.specular_texture, fs_in.TexCoords).rgb;
		} else {
			specular = light.specular * spec * texture(material.diffuse_texture, fs_in.TexCoords).rgb;
		}
	} else {
		ambient = light.ambient * material.ambient.rgb;
		diffuse = light.diffuse * diff * material.diffuse.rgb;
		if (useSpecularTexture && material.enableSpecularTexture) {
			specular = light.specular * spec * texture(material.specular_texture, fs_in.TexCoords).rgb;
		} else {
			specular = light.specular * material.specular.rgb;
		}
	}

	if (light.caster != 0) {
		// Point Light or Spot Light
		float distance = length(light.position - fs_in.FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}

	if (light.caster == 2) {
		// Spot Light
		float theta = dot(lightDir, normalize(-light.direction));
		float epsilon = light.cutoff - light.outerCutoff;
		float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

		ambient *= intensity;
		diffuse *= intensity;
		specular *= intensity;
	}

	return ambient + diffuse + specular;
}

void main() {

	vec3 norm = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	
	vec4 texel_diffuse = vec4(0.0);
	if (useDiffuseTexture && material.enableColorTexture) {
		texel_diffuse = texture(material.diffuse_texture, fs_in.TexCoords);
	} else {
		texel_diffuse = material.diffuse;
	}

	// 是否開啟光照
	if (!useLighting) {
		FragColor = texel_diffuse;
	} else {
		// 計算光照
		vec3 illumination = vec3(0.0f);

		for (int i = 0; i < NUM_LIGHTS; i++) {
			if (!lights[i].enable) {
				continue;
			}
			illumination += CalcLight(lights[i], norm, viewDir);
		}

		// 開啟自發光
		if (material.enableEmission && useEmission) {
			if (material.enableEmissionTexture) {
				illumination += texture(material.emission_texture, fs_in.TexCoords).rgb;
			} else {
				illumination += texel_diffuse.rgb * 1.5;
			}
		}

		if (useGamma) {
			illumination = pow(illumination, vec3(GammaValue));
		}

		FragColor = vec4(clamp(illumination, 0.0, 1.0), texel_diffuse.a);
	}
}