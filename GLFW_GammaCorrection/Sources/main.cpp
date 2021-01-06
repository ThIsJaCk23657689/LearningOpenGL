#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Headers/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../Headers/mstack.h"
#include "../Headers/shader.h"
#include "../Headers/camera.h"
#include "../Headers/model.h"
#include "../Headers/light.h"

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <random>

void showUI();
void geneObejectData();
void geneSphereData();
void drawFloor();
void drawCube();
void drawSphere();
void drawBox();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void proceessInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(char const* path);

// ========== Global Variable ==========

// Window parameters
const std::string WINDOW_TITLE = "Gamma Correction";
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// Matrix stack paramters
StackArray modelMatrix;

// Transform Matrices
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// Time parameters
float deltaTime = 0.0f;
float lastTime = 0.0f;

// Camera parameter
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraView = false;

// Light Parameters
Light dirLight(glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f), false);
std::vector<Light> pointLights = {
	Light(glm::vec3(8.0f, 3.0f, 8.0f), true),
	Light(glm::vec3(-15.0f, 7.5f, 13.0f), true),
	Light(glm::vec3(7.5f, 6.25f, -9.0f), true),
	Light(glm::vec3(-16.0f, 7.0f, -18.0f), true),
};
Light spotLight(camera.Position, camera.Position, false);

static bool useBlinnPhong = true;
static bool useLighting = true;
static bool useDiffuseTexture = true;
static bool useSpecularTexture = true;
static bool useEmission = true;
static bool useGamma = false;
static float GammaValue = 1.0f / 2.2f;

// Object Data
std::vector<float> cubeVertices;
std::vector<int> cubeIndices;
unsigned int cubeVAO, cubeVBO, cubeEBO;

std::vector<float> floorVertices;
std::vector<unsigned int> floorIndices;
unsigned int floorVAO, floorVBO, floorEBO;

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;
unsigned int sphereVAO, sphereVBO, sphereEBO;

// Texture parameter
unsigned int boxTexture, boxSpecularTexture, floorTexture;

int main(int argc, char* argv[]) {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 32);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD.\n");
		glfwTerminate();
		return -1;
	}

	// Initalize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	// Setting OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader myShader("Shaders/gamma.vs", "Shaders/gamma.fs");

	// Setting amount of boxes.
	std::default_random_engine generator(time(NULL));
	std::uniform_real_distribution<float> unif_b(-30.0, 30.0);
	
	std::vector<glm::vec3> boxposition;
	for (int i = 0; i < 20; i++) {
		boxposition.push_back(glm::vec3(unif_b(generator), 0.0f, unif_b(generator)));
	}

	// Create object data
	geneObejectData();

	// Loading textures
	floorTexture = loadTexture("Resources/Textures/wood.png");
	boxTexture = loadTexture("Resources/Textures/container2.png");
	boxSpecularTexture = loadTexture("Resources/Textures/container2_specular.png");

	// 1. Generate Frame buffer
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	
	// 2. Generate Depth Texture
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMap;
	glGenFramebuffers(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 3. Bind to the Frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// The main loop
	while (!glfwWindowShouldClose(window)) {

		// Calculate the deltaFrame
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Process Input (Moving camera)
		proceessInput(window);

		// Clear the buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		showUI();

		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 250.0f);

		// Enable Shader and setting view & projection matrix
		myShader.use();
		myShader.setMat4("view", view);
		myShader.setMat4("projection", projection);
		myShader.setVec3("viewPos", camera.Position);
		myShader.setBool("useBlinnPhong", useBlinnPhong);
		myShader.setBool("useLighting", useLighting);
		myShader.setBool("useDiffuseTexture", useDiffuseTexture);
		myShader.setBool("useSpecularTexture", useSpecularTexture);
		myShader.setBool("useEmission", useEmission); 
		myShader.setBool("useGamma", useGamma);
		myShader.setFloat("GammaValue", GammaValue);
		
		myShader.setInt("material.diffuse_texture", 0);
		myShader.setInt("material.specular_texture", 1);
		myShader.setInt("material.emission_texture", 2);

		myShader.setVec4("material.ambient", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		myShader.setVec4("material.diffuse", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
		myShader.setVec4("material.specular", glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
		myShader.setFloat("material.shininess", 64.0f);

		myShader.setVec3("lights[0].direction", dirLight.Direction);
		myShader.setVec3("lights[0].ambient", dirLight.Ambient);
		myShader.setVec3("lights[0].diffuse", dirLight.Diffuse);
		myShader.setVec3("lights[0].specular", dirLight.Specular);
		myShader.setBool("lights[0].enable", dirLight.Enable);
		myShader.setInt("lights[0].caster", dirLight.Caster);
		
		for (unsigned int i = 0; i < pointLights.size(); i++) {
			myShader.setVec3("lights[" + to_string(i) + "].position", pointLights[i].Position);
			myShader.setVec3("lights[" + to_string(i) + "].ambient", pointLights[i].Ambient);
			myShader.setVec3("lights[" + to_string(i) + "].diffuse", pointLights[i].Diffuse);
			myShader.setVec3("lights[" + to_string(i) + "].specular", pointLights[i].Specular);
			myShader.setFloat("lights[" + to_string(i) + "].constant", pointLights[i].Constant);
			myShader.setFloat("lights[" + to_string(i) + "].linear", pointLights[i].Linear);
			myShader.setFloat("lights[" + to_string(i) + "].quadratic", pointLights[i].Quadratic);
			myShader.setFloat("lights[" + to_string(i) + "].enable", pointLights[i].Enable);
			myShader.setInt("lights[" + to_string(i) + "].caster", pointLights[i].Caster);
		}

		spotLight.Position = camera.Position;
		spotLight.Direction = camera.Front;

		myShader.setVec3("lights[5].position", spotLight.Position);
		myShader.setVec3("lights[5].direction", spotLight.Direction);
		myShader.setVec3("lights[5].ambient", spotLight.Ambient);
		myShader.setVec3("lights[5].diffuse", spotLight.Diffuse);
		myShader.setVec3("lights[5].specular", spotLight.Specular);
		myShader.setFloat("lights[5].cutoff", glm::cos(glm::radians(spotLight.Cutoff)));
		myShader.setFloat("lights[5].outerCutoff", glm::cos(glm::radians(spotLight.OuterCutoff)));
		myShader.setFloat("lights[5].constant", spotLight.Constant);
		myShader.setFloat("lights[5].linear", spotLight.Linear);
		myShader.setFloat("lights[5].quadratic", spotLight.Quadratic);
		myShader.setBool("lights[5].enable", spotLight.Enable);
		myShader.setInt("lights[5].caster", spotLight.Caster);

		// Draw Floor
		myShader.setBool("material.enableColorTexture", true);
		myShader.setBool("material.enableSpecularTexture", false);
		myShader.setBool("material.enableEmission", false);
		myShader.setBool("material.enableEmissionTexture", false);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NULL);
		myShader.setFloat("material.shininess", 64.0f);
		myShader.setMat4("model", modelMatrix.top());
		drawFloor();

		// Draw boxes
		myShader.setBool("material.enableColorTexture", true);
		myShader.setBool("material.enableSpecularTexture", true);
		myShader.setBool("material.enableEmission", false);
		myShader.setBool("material.enableEmissionTexture", false);
		modelMatrix.push();
			for (unsigned int i = 0; i < boxposition.size(); i++) {
				modelMatrix.push();
					modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(boxposition[i].x, 0.5f, boxposition[i].z)));
					myShader.setFloat("material.shininess", 64.0f);
					myShader.setMat4("model", modelMatrix.top());
					drawBox();
				modelMatrix.pop();
			}
		modelMatrix.pop();

		// draw light ball
		myShader.setBool("material.enableColorTexture", false);
		myShader.setBool("material.enableEmission", true);
		for (unsigned int i = 0; i < pointLights.size(); i++) {
			if (!pointLights[i].Enable) {
				continue;
			}
			modelMatrix.push();
				modelMatrix.save(glm::translate(modelMatrix.top(), pointLights[i].Position));
				modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.5f)));
				myShader.setVec4("material.diffuse", glm::vec4(pointLights[i].Diffuse.x, pointLights[i].Diffuse.y, pointLights[i].Diffuse.z, 1.0f));
				myShader.setVec4("material.specular", glm::vec4(pointLights[i].Specular.x, pointLights[i].Specular.y, pointLights[i].Specular.z, 1.0f));
				myShader.setFloat("material.shininess", 32.0f);
				myShader.setMat4("model", modelMatrix.top());
				drawSphere();
			modelMatrix.pop();
		}
		myShader.setBool("material.enableEmission", false);

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);

	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);
	
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereEBO);

	// clean up
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void showUI() {
	ImGui::Begin("Control Panel");
	ImGuiTabBarFlags tab_bar_flags = ImGuiBackendFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
		if (ImGui::BeginTabItem("Camera")) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Camera");
			ImGui::Text("Position = (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
			ImGui::Text("Front = (%.2f, %.2f, %.2f)", camera.Front.x, camera.Front.y, camera.Front.z);
			ImGui::Text("Right = (%.2f, %.2f, %.2f)", camera.Right.x, camera.Right.y, camera.Right.z);
			ImGui::Text("Up = (%.2f, %.2f, %.2f)", camera.Up.x, camera.Up.y, camera.Up.z);
			ImGui::Text("Pitch = %.2f deg, Yaw = %.2f deg", camera.Pitch, camera.Yaw);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Projection")) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Perspective Projection");
			ImGui::Text("Parameters");
			ImGui::BulletText("FoV = %.2f deg, Aspect = %.2f", camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT);
			ImGui::Spacing();

			if (ImGui::TreeNode("Projection Matrix")) {
				ImGui::Columns(4, "mycolumns");
				ImGui::Separator();
				for (int i = 0; i < 4; i++) {
					ImGui::Text("%.2f", projection[0][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", projection[1][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", projection[2][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", projection[3][i]); ImGui::NextColumn();
					ImGui::Separator();
				}
				ImGui::Columns(1);

				ImGui::TreePop();
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Illumination")) {
			ImGui::Text("Lighting Model: %s", useBlinnPhong ? "Blinn-Phong" : "Phong");
			ImGui::Checkbox("Lighting", &useLighting);
			ImGui::Checkbox("DiffuseTexture", &useDiffuseTexture);
			ImGui::Checkbox("SpecularTexture", &useSpecularTexture);
			ImGui::Checkbox("Emission", &useEmission);
			ImGui::Checkbox("Gamma Correction", &useGamma);
			ImGui::SliderFloat("Gamma Value", &GammaValue, 1.0f / 2.2f, 2.2f);
			ImGui::Spacing();

			if (ImGui::TreeNode("Direction Light")) {
				ImGui::SliderFloat3("Direction", (float*)&dirLight.Direction, -10.0f, 10.0f);
				ImGui::SliderFloat3("Ambient", (float*)&dirLight.Ambient, 0.0f, 1.0f);
				ImGui::SliderFloat3("Diffuse", (float*)&dirLight.Diffuse, 0.0f, 1.0f);
				ImGui::SliderFloat3("Specular", (float*)&dirLight.Specular, 0.0f, 1.0f);
				ImGui::Checkbox("Enable", &dirLight.Enable);
				ImGui::TreePop();
			}
			ImGui::Spacing();

			for (unsigned int i = 0; i < pointLights.size(); i++) {
				std::stringstream ss;
				ss << i;
				std::string index = ss.str();

				if (ImGui::TreeNode(std::string("Point Light " + index).c_str())) {
					if (i != 4) {
						// ROV的光，因為位置是鎖定在ROV上，所以這邊不給調整
						ImGui::SliderFloat3(std::string("Position").c_str(), (float*)&pointLights[i].Position, -50.0f, 50.0f);
					}

					ImGui::SliderFloat3(std::string("Ambient").c_str(), (float*)&pointLights[i].Ambient, 0.0f, 1.0f);
					ImGui::SliderFloat3(std::string("Diffuse").c_str(), (float*)&pointLights[i].Diffuse, 0.0f, 1.0f);

					if (i != 4) {
						// ROV的Specular，不給調整，看起來才不會怪怪的
						ImGui::SliderFloat3(std::string("Specular").c_str(), (float*)&pointLights[i].Specular, 0.0f, 1.0f);
					}

					ImGui::SliderFloat(std::string("Linear").c_str(), (float*)&pointLights[i].Linear, 0.00014f, 0.7f);
					ImGui::SliderFloat(std::string("Quadratic").c_str(), (float*)&pointLights[i].Quadratic, 0.00007, 0.5f);
					ImGui::Checkbox(std::string("Enable").c_str(), &pointLights[i].Enable);
					ImGui::Spacing();
					ImGui::TreePop();
				}
				ImGui::Spacing();
			}

			if (ImGui::TreeNode("Spot Light")) {
				ImGui::Text("Position: (%.2f, %.2f, %.2f)", spotLight.Position.x, spotLight.Position.y, spotLight.Position.z);
				ImGui::Text("Direction: (%.2f, %.2f, %.2f)", spotLight.Direction.x, spotLight.Direction.y, spotLight.Direction.z);
				ImGui::SliderFloat3("Ambient", (float*)&spotLight.Ambient, 0.0f, 1.0f);
				ImGui::SliderFloat3("Diffuse", (float*)&spotLight.Diffuse, 0.0f, 1.0f);
				ImGui::SliderFloat3("Specular", (float*)&spotLight.Specular, 0.0f, 1.0f);
				ImGui::SliderFloat("Linear", (float*)&spotLight.Linear, 0.00014f, 0.7f);
				ImGui::SliderFloat("Quadratic", (float*)&spotLight.Quadratic, 0.00007f, 0.5f);
				ImGui::SliderFloat("Cutoff", (float*)&spotLight.Cutoff, 0.0f, spotLight.OuterCutoff - 1);
				ImGui::SliderFloat("OuterCutoff", (float*)&spotLight.OuterCutoff, spotLight.Cutoff + 1, 40.0f);
				ImGui::Checkbox("Enable", &spotLight.Enable);
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::End();
}

void geneObejectData() {
	// ========== Generate Cube vertex data ==========
	cubeVertices = {
		// Positions			// Normals 			// Texture coords
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,

		0.5f,  0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	};
	cubeIndices = {
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
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================


	// ========== Generate floor vertex data ==========
	floorVertices = {
		// Positions			// Normals			// Texture Coords
		-100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f,  0.0f,
		 100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f, 25.0f,
		 100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f, 25.0f,
		-100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f,  0.0f,
	};
	floorIndices = {
		0, 1, 2,
		0, 2, 3,
	};
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorEBO);
	glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(float), floorVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorIndices.size() * sizeof(unsigned int), floorIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================

	// ========== Generate sphere vertex data ==========
	geneSphereData();
	// ==================================================
}

void geneSphereData() {
	float radius = 1.0f;
	unsigned int latitude = 30;
	unsigned int longitude = 30;

	for (int i = 0; i <= latitude; i++) {
		float theta = i * M_PI / latitude;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);
		for (int j = 0; j <= longitude; j++) {
			float phi = j * 2.0f * M_PI / longitude;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			float x = cosPhi * sinTheta;
			float y = cosTheta;
			float z = sinPhi * sinTheta;

			sphereVertices.push_back(radius * x);
			sphereVertices.push_back(radius * y);
			sphereVertices.push_back(radius * z);

			// Generate normal vectors
			glm::vec3 normal = glm::vec3(2 * radius * x, 2 * radius * y, 2 * radius * z);
			normal = glm::normalize(normal);
			sphereVertices.push_back(normal.x);
			sphereVertices.push_back(normal.y);
			sphereVertices.push_back(normal.z);

			// Generate texture coordinate
			float u = 1 - (j / longitude);
			float v = 1 - (i / latitude);
			sphereVertices.push_back(u);
			sphereVertices.push_back(-v);
		}
	}

	for (int i = 0; i < latitude; i++) {
		for (int j = 0; j < longitude; j++) {
			int first = (i * (longitude + 1)) + j;
			int second = first + longitude + 1;

			sphereIndices.push_back(first);
			sphereIndices.push_back(second);
			sphereIndices.push_back(first + 1);

			sphereIndices.push_back(second);
			sphereIndices.push_back(second + 1);
			sphereIndices.push_back(first + 1);
		}
	}

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

void drawFloor() {
	modelMatrix.push();
		glBindVertexArray(floorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	modelMatrix.pop();
}

void drawCube() {
	modelMatrix.push();
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	modelMatrix.pop();
}

void drawSphere() {
	modelMatrix.push();
	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawBox() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, boxTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, boxSpecularTexture);
	drawCube();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {

	// Set new width and height
	SCR_WIDTH = width;
	SCR_HEIGHT = height;

	// Reset projection matrix and viewport
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 250.0f);
	glViewport(0, 0, width, height);
}

void proceessInput(GLFWwindow* window) {
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
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.MovementSpeed = 25.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		camera.MovementSpeed = 10.0f;
	}
}

// Handle the key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// Only handle press events
	if (action == GLFW_RELEASE) {
		return;
	}

	// Exit the program
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}

	// 手電筒開關
	if (key == GLFW_KEY_F) {
		if (spotLight.Enable) {
			spotLight.Enable = false;
		} else {
			spotLight.Enable = true;
		}
	}

	// Blinn-Phong 與 Phong 光照模型的切換
	if (key == GLFW_KEY_L) {
		if (useBlinnPhong) {
			useBlinnPhong = false;
		} else {
			useBlinnPhong = true;
		}
	}
}

// Handle mouse movement (cursor's position)
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

// Handle mouse button (like: left middle right)
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

// Handle mouse scroll
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

// Loading Texture
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
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