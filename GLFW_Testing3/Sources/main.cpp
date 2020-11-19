#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Headers/shader.h"
#include "../Headers/camera.h"
#include "../Headers/model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void proceessInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraView = false;
bool enableFlash = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 pointLightPosition = glm::vec3(5.0f, 5.0f, 5.0f);

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
	glfwSetKeyCallback(window, keyCallback);

	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD.\n");
		glfwTerminate();
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	glEnable(GL_DEPTH_TEST);

	Shader ourShader("Shaders\\model_loading.vs", "Shaders\\model_loading.fs");
	Shader lightCubeShader("Shaders\\lightcube.vs", "Shaders\\lightcube.fs");

	Model ourModel("Resources\\objects\\nanosuit\\nanosuit.obj");

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

	unsigned int lightCubeVAO, VBO, EBO;
	glGenVertexArrays(1, &lightCubeVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(lightCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Draw in wireframe
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Initalize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		proceessInput(window);

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ourShader.use();
		ourShader.setVec3("viewPos", camera.Position);

		// Define Direct Light Parameters
		ourShader.setVec3("dirlight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		ourShader.setVec3("dirlight.color", glm::vec3(1.0f, 0.0f, 0.0f));
		ourShader.setVec3("dirlight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		ourShader.setVec3("dirlight.diffuse", glm::vec3(0.05f, 0.05f, 0.05f));
		ourShader.setVec3("dirlight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
		
		ourShader.setVec3("pointlight.position", pointLightPosition);
		ourShader.setVec3("pointlight.color", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setVec3("pointlight.ambient", glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f);
		ourShader.setVec3("pointlight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setVec3("pointlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setFloat("pointlight.constant", 1.0f);
		ourShader.setFloat("pointlight.linear", 0.022f);
		ourShader.setFloat("pointlight.quadratic", 0.0019f);

		// Define Spotlight Parameters
		ourShader.setVec3("spotlight.position", camera.Position);
		ourShader.setVec3("spotlight.direction", camera.Front);
		ourShader.setVec3("spotlight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		ourShader.setVec3("spotlight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setVec3("spotlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setFloat("spotlight.constant", 1.0f);
		ourShader.setFloat("spotlight.linear", 0.022f);
		ourShader.setFloat("spotlight.quadratic", 0.0019f);
		ourShader.setBool("spotlight.enableFlash", enableFlash);

		glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		static float color[] = { spotlightColor.x, spotlightColor.y, spotlightColor.z };
		static float lightOuterCutoff = 17.5f;
		ImGui::Begin("Spotlight Setting");
		ImGui::ColorPicker3("Color", color);
		ImGui::SliderFloat("Outer cutoff", &lightOuterCutoff, 10, 100);
		ImGui::End();
		spotlightColor = glm::vec3(color[0], color[1], color[2]);
		ourShader.setVec3("spotlight.color", spotlightColor);
		ourShader.setFloat("spotlight.cutoff", glm::cos(glm::radians(lightOuterCutoff - 5.0f)));
		ourShader.setFloat("spotlight.outerCutoff", glm::cos(glm::radians(lightOuterCutoff)));

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		float angle = 40.0f * currentFrame;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(angle / 10), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		ourShader.setMat4("model", model);
		ourShader.setMat3("normalModel", glm::mat3(glm::transpose(glm::inverse(model))));
		ourModel.Draw(ourShader);

		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		glBindVertexArray(lightCubeVAO);
		
		model = glm::mat4(1.0f);
		pointLightPosition = glm::vec3(sin(currentFrame) * 5, pointLightPosition.y, cos(currentFrame) * 5);
		model = glm::translate(model, pointLightPosition);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", model);
		lightCubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
	// only handle press events
	if (action == GLFW_RELEASE) {
		return;
	}

	if (key == GLFW_KEY_F) {
		enableFlash = !enableFlash;
	};
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