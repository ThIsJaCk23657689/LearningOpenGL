#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void proceessInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float PI = 3.141569;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraView = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 5.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

int main(void) {

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW.\n");
		return -1;
	}

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

	Shader ourShader("Shaders\\shader.vs", "Shaders\\shader.fs");
	Shader lightShader("Shaders\\lightcube.vs", "Shaders\\lightcube.fs");

	// Initalize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO	&io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	float vertices[] = {
		// positions			// texture coords		// normal vector
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,				0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f,				0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f,				0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,				0.0f, 0.0f, 1.0f,

		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,				1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f,				1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f,				1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f,		0.0f, 1.0f,				1.0f, 0.0f, 0.0f,

		-0.5f, 0.5f, 0.5f,		1.0f, 1.0f,				-1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		1.0f, 0.0f,				-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f,				-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f,				-1.0f, 0.0f, 0.0f,

		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,				0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f,				0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 0.0f,				0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,				0.0f, 1.0f, 0.0f,

		0.5f, -0.5f, 0.5f,		1.0f, 1.0f,				0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,				0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f,				0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f,				0.0f, -1.0f, 0.0f,
			
		0.5f, 0.5f, -0.5f,		1.0f, 1.0f,				0.0f, 0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,				0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f,				0.0f, 0.0f, -1.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f,				0.0f, 0.0f, -1.0f,
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

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	unsigned int texture1, texture2;
	int width, height, nrChannels;
	unsigned char* data;

	// stbi_set_flip_vertically_on_load(true);

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("Resources\\Textures\\container.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("Resources\\Textures\\awesomeface.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		proceessInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_WIDTH, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		ourShader.use();
		// ourShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		// lightPos = glm::vec3(cos(currentFrame), sin(currentFrame), cos(currentFrame));
		// ourShader.setVec3("lightColor", lightColor);
		ourShader.setVec3("viewPos", camera.Position);
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);
		ourShader.setInt("texture1", 0);
		ourShader.setInt("texture2", 1);

		//lightColor.x = sin(currentFrame * 2.0f) / 2 + 1.0f;
		//lightColor.y = sin(currentFrame * 0.7f) / 2 + 1.0f;
		//lightColor.z = sin(currentFrame * 1.3f) / 2 + 1.0f;

		static float lightAmbient = 1.0f;
		static float lightDiffuse = 1.0f;
		static float lightSpecular = 1.0f;
		ImGui::Begin("Light Setting");
		ImGui::SliderFloat("ambient", &lightAmbient, 0, 1);
		ImGui::SliderFloat("diffuse", &lightDiffuse, 0, 1);
		ImGui::SliderFloat("specular", &lightSpecular, 0, 1);
		ImGui::End();
		ourShader.setVec3("light.ambient", lightColor * glm::vec3(lightAmbient));
		ourShader.setVec3("light.diffuse", lightColor * glm::vec3(lightDiffuse));
		ourShader.setVec3("light.specular", glm::vec3(lightSpecular));

		glm::vec3 materialAmbientVec(0.19225f, 0.19225f, 0.19225f);
		glm::vec3 materialDiffuseVec(0.50754f, 0.50754f, 0.50754f);
		glm::vec3 materialSpecularVec(0.508273f, 0.508273f, 0.508273f);
		static float materialAmbient[] = { materialAmbientVec.x, materialAmbientVec.y, materialAmbientVec.z };
		static float materialDiffuse[] = { materialDiffuseVec.x, materialDiffuseVec.y, materialDiffuseVec.z };
		static float materialSpecular[] = { materialSpecularVec.x, materialSpecularVec.y, materialSpecularVec.z };
		static float materialShininess = 32.0f;
		ImGui::Begin("Material Setting");
		ImGui::SliderFloat3("ambient", materialAmbient, 0, 1);
		ImGui::SliderFloat3("diffuse", materialDiffuse, 0, 1);
		ImGui::SliderFloat3("specular", materialSpecular, 0, 1);
		ImGui::SliderFloat("shininess", &materialShininess, 0, 1024);
		ImGui::End();
		materialAmbientVec = glm::vec3(materialAmbient[0], materialAmbient[1], materialAmbient[2]);
		materialDiffuseVec = glm::vec3(materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
		materialSpecularVec = glm::vec3(materialSpecular[0], materialSpecular[1], materialSpecular[2]);
		ourShader.setVec3("material.ambient", materialAmbientVec);
		ourShader.setVec3("material.diffuse", materialDiffuseVec);
		ourShader.setVec3("material.specular", materialSpecularVec);
		ourShader.setFloat("material.shininess", materialShininess);

		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * (i + 1) * currentFrame;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.5f, 0.3f, 0.5f));
			ourShader.setMat4("model", model);
			ourShader.setMat3("normalModel", glm::mat3(glm::transpose(glm::inverse(model))));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// render GUI
		static float rotation = 0.0f;
		static float translation[] = { lightPos.x, lightPos.y, lightPos.z };
		float color[4] = { lightColor.x, lightColor.y ,lightColor.z, 1.0f };

		ImGui::Begin("Position / Color");
		ImGui::SliderFloat("Rotation", &rotation, 0, 360);
		ImGui::SliderFloat3("Position", translation, -10.0, 3.0);
		ImGui::ColorPicker3("Color", color);
		ImGui::End();

		lightPos = glm::vec3(translation[0], translation[1], translation[2]);
		ourShader.setVec3("light.position", lightPos);

		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f));
		lightColor = glm::vec3(color[0], color[1], color[2]);

		lightShader.use();
		lightShader.setMat4("model", model);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		lightShader.setVec3("lightColor", lightColor);
		glBindVertexArray(lightVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
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
	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
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