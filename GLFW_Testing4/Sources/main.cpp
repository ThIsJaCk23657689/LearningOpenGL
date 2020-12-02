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
#include "../Headers/model.h"

#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void proceessInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<std::string> faces);
glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar);
glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far);

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraView = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

unsigned int framebuffer, texColorBuffer, rbo;

int main(int argc, char *argv[]) {

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
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD.\n");
		glfwTerminate();
		return -1;
	}

	Shader ourShader("Shaders\\deapth_testing.vs", "Shaders\\deapth_testing.fs");
	Shader screenShader("Shaders\\framebuffer_screen.vs", "Shaders\\framebuffer_screen.fs");
	Shader cubemapShader("Shaders\\cubemap.vs", "Shaders\\cubemap.fs");
	Shader reflectShader("Shaders\\reflection.vs", "Shaders\\reflection.fs");
	// Shader singleShader("Shaders\\deapth_testing.vs", "Shaders\\outlining.fs");

	// Initalize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	float cubeVertices[] = {
		// positions			// Normal				// texture coords
		 0.5f,  0.5f,  0.5f,	 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		-0.5F,  0.5f,  0.5f,	 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,		 1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,		 1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		 1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		0.5f,  0.5f, -0.5f,		 1.0f, 0.0f, 0.0f,		1.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	 0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	 0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	 0.0f, 1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	 0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
		
		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,		1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,		0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
	};

	unsigned int cubeIndices[] = {
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

	float planeVertices[] = {
		// Position				// Normal			// Texture Coords
		 5.0f, -0.5f,  5.0f,	0.0f, 1.0f, 0.0f,	2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,	0.0f, 1.0f, 0.0f,	0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,	0.0f, 1.0f, 0.0f,	2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,	0.0f, 1.0f, 0.0f,	0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,	0.0f, 1.0f, 0.0f,	2.0f, 2.0f,
	};

	float grassVertices[] = {
		 0.0,  0.5, 0.0,		0.0f, 0.0f, 1.0f,	0.0, 0.0,
		 0.0, -0.5, 0.0,		0.0f, 0.0f, 1.0f,	0.0, 1.0,
		 1.0, -0.5, 0.0,		0.0f, 0.0f, 1.0f,	1.0, 1.0,

		 0.0,  0.5, 0.0,		0.0f, 0.0f, 1.0f,	0.0, 0.0,
		 1.0, -0.5, 0.0,		0.0f, 0.0f, 1.0f,	1.0, 1.0,
		 1.0,  0.5, 0.0,		0.0f, 0.0f, 1.0f,	1.0, 0.0,
	};

	float quadVertices[] = {
		-1.0f,  1.0f,		0.0f, 1.0f,
		-1.0f, -1.0f,		0.0f, 0.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,

		-1.0f,  1.0f,		0.0f, 1.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,
		 1.0f,  1.0f,		1.0f, 1.0f
	};

	unsigned int cubeVAO, cubeVBO, cubeEBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int grassVAO, grassVBO;
	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);
	glBindVertexArray(grassVAO);
		glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	// Create our frame buffer.
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Create texture (a color) to attach our frame buffer.
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error: Framebuffer is not completed!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int cubeTexture = loadTexture("Resources\\Textures\\marble.jpg");
	unsigned int cubeTexture2 = loadTexture("Resources\\Textures\\container.jpg");
	unsigned int floorTexture = loadTexture("Resources\\Textures\\metal.png");
	unsigned int grassTexture = loadTexture("Resources\\Textures\\window.png");

	vector<std::string> faces{
		"Resources/Textures/skybox/right.jpg",
		"Resources/Textures/skybox/left.jpg",
		"Resources/Textures/skybox/top.jpg",
		"Resources/Textures/skybox/bottom.jpg",
		"Resources/Textures/skybox/front.jpg",
		"Resources/Textures/skybox/back.jpg",
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	vector<glm::vec3> windowsPosition{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3( 1.5f, 0.0f,  0.51f),
		glm::vec3( 0.0f, 0.0f,  0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3( 0.5f, 0.0f, -0.6f),
	};

	ourShader.use();
	ourShader.setInt("texture1", 0);

	reflectShader.use();
	reflectShader.setInt("skybox", 0);
	reflectShader.setInt("texture1", 1);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		proceessInput(window);

		// sort the transparent windows before rendering
		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windowsPosition.size(); i++) {
			float distance = glm::length(camera.Position - windowsPosition[i]);
			sorted[distance] = windowsPosition[i];
		}

		// 1. PhaseOne
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glm::mat4 model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		// projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		projection = GetPerspectiveProjMatrix(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
		//projection = GetOrthoProjMatrix(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

		ourShader.use();
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		reflectShader.use();
		reflectShader.setMat4("view", view);
		reflectShader.setMat4("projection", projection);
		reflectShader.setVec3("cameraPos", camera.Position);

		// Draw Skybox
		glDepthFunc(GL_LEQUAL);
		cubemapShader.use();
		glm::mat4 view_skybox = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		cubemapShader.setMat4("view", view_skybox);
		cubemapShader.setMat4("projection", projection);
		glBindVertexArray(cubeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			model = glm::mat4(1.0f);
			cubemapShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// Draw floor
		ourShader.use();
		glBindVertexArray(planeVAO);
			glBindTexture(GL_TEXTURE_2D, floorTexture);
			model = glm::mat4(1.0f);
			ourShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// Draw Cube
		reflectShader.use();
		glEnable(GL_CULL_FACE);
		glBindVertexArray(cubeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, cubeTexture);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-1.0f, 0.001f, -1.0f));
			reflectShader.setMat4("model", model);
			reflectShader.setMat3("normalModel", glm::mat3(glm::transpose(glm::inverse(model))));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(2.0f, 0.001f, 0.0f));
			reflectShader.setMat4("model", model);
			reflectShader.setMat3("normalModel", glm::mat3(glm::transpose(glm::inverse(model))));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);

		// Draw Windows
		ourShader.use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(grassVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			ourShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glBindVertexArray(0);
		glDisable(GL_BLEND);

		

		
		// 2. Phase Two
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &framebuffer);
	
	// clean up
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// only handle press events
	if (action == GLFW_RELEASE) {
		return;
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

unsigned int loadTexture(char const* path) {
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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

unsigned int loadCubemap(vector<std::string> faces) {

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char*  data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		} else {
			std::cout << "Failed to load Cubemap texture at path:" << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return textureID;
}

glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar) {
	
	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 1 / (tan(fovy / 2) * ascept);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = 0;

	proj[0][1] = 0;
	proj[1][1] = 1 / (tan(fovy / 2));
	proj[2][1] = 0;
	proj[3][1] = 0;

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -(zfar + znear) / (zfar - znear);
	proj[3][2] = (-2 * zfar * znear) / (zfar - znear);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = -1;
	proj[3][3] = 0;

	return proj;
}

glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far) {
	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 2 / (right - left);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = - (right + left) / (right - left);

	proj[0][1] = 0;
	proj[1][1] = 2 / (top - bottom);
	proj[2][1] = 0;
	proj[3][1] = - (top + bottom) / (top - bottom);

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -2 / (far - near);
	proj[3][2] = - (far + near) / (far - near);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = 0;
	proj[3][3] = 1;

	return proj;
}