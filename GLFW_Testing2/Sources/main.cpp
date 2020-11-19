#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Headers/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/shader.h"
#include "../Headers/camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void proceessInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraView = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f, 0.2f, 2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f, 2.0f, -12.0f),
	glm::vec3(0.0f, 0.0f, -3.0f)
};

glm::vec3 pointLightColors[] = {
	glm::vec3(0.1f, 0.1f, 0.1f),
	glm::vec3(0.1f, 0.1f, 0.1f),
	glm::vec3(0.1f, 0.1f, 0.1f),
	glm::vec3(0.9f, 0.0f, 0.0f)
};

int main(void) {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Program", NULL, NULL);
	if (!window) {
		fprintf(stderr, "Failed to create GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD.\n");
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader lightingShader("Shaders\\shader.vs", "Shaders\\shader.fs");
	Shader lightCubeShader("Shaders\\lightcube.vs", "Shaders\\lightcube.fs");

	// Initalize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	float vertices[] = {
		// positions			// normal vector		// texture coords
		0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		0.5f, 0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,

		-0.5f, 0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		0.0f, 1.0f,

		0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,

		0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		0.0f, 1.0f,

		0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23,
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f),
	};

	unsigned int cubeVAO, VBO, EBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
;
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int diffuseMap = loadTexture("Resources\\Textures\\container2.png");
	unsigned int specularMap = loadTexture("Resources\\Textures\\container2_specular.png");
	unsigned int emissionMap = loadTexture("Resources\\Textures\\matrix.jpg");

	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);
	lightingShader.setInt("material.emission", 2);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		proceessInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		lightingShader.use();
		lightingShader.setFloat("time", currentFrame);
		lightingShader.setVec3("viewPos", camera.Position);

		// Define Direct Light Parameters
		lightingShader.setVec3("dirlight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		lightingShader.setVec3("dirlight.color", glm::vec3(1.0f, 0.0f, 0.0f));
		lightingShader.setVec3("dirlight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		lightingShader.setVec3("dirlight.diffuse", glm::vec3(0.05f, 0.05f, 0.05f));
		lightingShader.setVec3("dirlight.specular", glm::vec3(0.2f, 0.2f, 0.2f));

		// Define Point Light Parameters
		lightingShader.setVec3("pointlights[0].position", pointLightPositions[0]);
		lightingShader.setVec3("pointlights[0].color", pointLightColors[0]);
		lightingShader.setVec3("pointlights[0].ambient", pointLightColors[0] * 0.1f);
		lightingShader.setVec3("pointlights[0].diffuse", pointLightColors[0]);
		lightingShader.setVec3("pointlights[0].specular", pointLightColors[0]);
		lightingShader.setFloat("pointlights[0].constant", 1.0f);
		lightingShader.setFloat("pointlights[0].linear", 0.14f);
		lightingShader.setFloat("pointlights[0].quadratic", 0.07f);

		lightingShader.setVec3("pointlights[1].position", pointLightPositions[1]);
		lightingShader.setVec3("pointlights[1].color", pointLightColors[1]);
		lightingShader.setVec3("pointlights[1].ambient", pointLightColors[1] * 0.1f);
		lightingShader.setVec3("pointlights[1].diffuse", pointLightColors[1]);
		lightingShader.setVec3("pointlights[1].specular", pointLightColors[1]);
		lightingShader.setFloat("pointlights[1].constant", 1.0f);
		lightingShader.setFloat("pointlights[1].linear", 0.14f);
		lightingShader.setFloat("pointlights[1].quadratic", 0.07f);

		lightingShader.setVec3("pointlights[2].position", pointLightPositions[2]);
		lightingShader.setVec3("pointlights[2].color", pointLightColors[2]);
		lightingShader.setVec3("pointlights[2].ambient", pointLightColors[2] * 0.1f);
		lightingShader.setVec3("pointlights[2].diffuse", pointLightColors[2]);
		lightingShader.setVec3("pointlights[2].specular", pointLightColors[2]);
		lightingShader.setFloat("pointlights[2].constant", 1.0f);
		lightingShader.setFloat("pointlights[2].linear", 0.22f);
		lightingShader.setFloat("pointlights[2].quadratic", 0.20f);

		lightingShader.setVec3("pointlights[3].position", pointLightPositions[3]);
		lightingShader.setVec3("pointlights[3].color", pointLightColors[3]);
		lightingShader.setVec3("pointlights[3].ambient", pointLightColors[3] * 0.1f);
		lightingShader.setVec3("pointlights[3].diffuse", pointLightColors[3]);
		lightingShader.setVec3("pointlights[3].specular", pointLightColors[3]);
		lightingShader.setFloat("pointlights[3].constant", 1.0f);
		lightingShader.setFloat("pointlights[3].linear", 0.14f);
		lightingShader.setFloat("pointlights[3].quadratic", 0.07f);

		// Define Spotlight Parameters
		lightingShader.setVec3("spotlight.position", camera.Position);
		lightingShader.setVec3("spotlight.direction", camera.Front);
		lightingShader.setVec3("spotlight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		lightingShader.setVec3("spotlight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setVec3("spotlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setFloat("spotlight.constant", 1.0f);
		lightingShader.setFloat("spotlight.linear", 0.09f);
		lightingShader.setFloat("spotlight.quadratic", 0.032f);

		glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		static float color[] = { spotlightColor.x, spotlightColor.y, spotlightColor.z };
		static float lightOuterCutoff = 17.5f;
		ImGui::Begin("Spotlight Setting");
		ImGui::ColorPicker3("Color", color);
		ImGui::SliderFloat("Outer cutoff", &lightOuterCutoff, 10, 100);
		ImGui::End();
		spotlightColor = glm::vec3(color[0], color[1], color[2]);
		lightingShader.setVec3("spotlight.color", spotlightColor);
		lightingShader.setFloat("spotlight.cutoff", glm::cos(glm::radians(lightOuterCutoff - 5.0f)));
		lightingShader.setFloat("spotlight.outerCutoff", glm::cos(glm::radians(lightOuterCutoff)));

		static float materialShininess = 64.0f;
		static float materialEmission = 0.0f;
		ImGui::Begin("Material Setting");
		ImGui::SliderFloat("shininess", &materialShininess, 0, 1024);
		ImGui::SliderFloat("emission", &materialEmission, 0, 4);
		ImGui::End();
		lightingShader.setFloat("material.shininess", materialShininess);
		lightingShader.setFloat("material.intensity", materialEmission);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_WIDTH, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, emissionMap);

		glm::mat4 model = glm::mat4(1.0f);
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			float angle = 20.0f * (i + 1) * currentFrame;
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.5f, 0.3f, 0.5f));
			lightingShader.setMat4("model", model);
			lightingShader.setMat3("normalModel", glm::mat3(glm::transpose(glm::inverse(model))));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		

		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		glBindVertexArray(lightCubeVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			lightCubeShader.setMat4("model", model);
			lightCubeShader.setVec3("lightColor", pointLightColors[i]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// clean up
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void proceessInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	if (moveCameraView) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		moveCameraView = true;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		moveCameraView = false;
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const *path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1) {
			format = GL_RED;
		}
		else if (nrComponents == 3) {
			format = GL_RGB;
		}
		else if (nrComponents == 4) {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load texture at path:" << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}