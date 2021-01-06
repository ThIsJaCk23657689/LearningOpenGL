#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

const glm::vec3 AMBIENT = glm::vec3(0.1f);
const glm::vec3 DIFFUSE = glm::vec3(0.7f);
const glm::vec3 SPECULAR = glm::vec3(0.4f);
const glm::vec3 EMISSION = glm::vec3(0.0f);

const float SHININESS = 64.0f;


class Object
{
public:
	std::vector<float> Vertices;
	std::vector<int> Indices;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
	glm::vec3 Emission;
	float Shininess;

	Object(bool enableDiffuseTexture = true, bool enableSpecularTexture = false, bool enableEmissionTexture = false) : Ambient(AMBIENT), Diffuse(DIFFUSE), Specular(SPECULAR), Emission(EMISSION), Shininess(SHININESS) {
		
	}
private:
};

#endif // !OBJECT_H