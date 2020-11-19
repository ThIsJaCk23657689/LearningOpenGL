#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Movement{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVactors();
	}

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVactors();
	}

	glm::mat4 GetViewMatrix() {
		// return glm::lookAt(Position, Position + Front, WorldUp);
		return calculate_lookAt_matrix(Position, Position + Front, WorldUp);
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD) {
			Position += Front * velocity;
		}
		if (direction == BACKWARD) {
			Position -= Front * velocity;
		}
		if (direction == LEFT) {
			Position -= Right * velocity;
		}
		if (direction == RIGHT) {
			Position += Right * velocity;
		}
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// fprintf(stderr, "Pitch�G%.2f, Yaw�G%.2f\t", Yaw, Pitch);

		if (constrainPitch) {
			if (Pitch > 89.0f) {
				Pitch = 89.0f;
			}
			if (Pitch < -89.0f) {
				Pitch = -89.0f;
			}
		}

		updateCameraVactors();
	}

	void ProcessMouseScroll(float yoffset) {
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f) {
			Zoom = 1.0f;
		}
		if (Zoom > 45.0f) {
			Zoom = 45.0f;
		}
	}

private:
	void updateCameraVactors() {
		glm::mat4 rotateMatrix = glm::mat4(1.0f);
		rotateMatrix = glm::rotate(rotateMatrix, glm::radians(-Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		rotateMatrix = glm::rotate(rotateMatrix, glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::vec4 front = rotateMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
		// fprintf(stderr, "Front:(%.2f, %.2f, %.2f)\n", front.x, front.y, front.z);

		Front = glm::normalize(glm::vec3(front.x, front.y, front.z));
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

	glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
		glm::vec3 zaxis = glm::normalize(position - target);
		glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
		glm::vec3 yaxis = glm::normalize(glm::cross(zaxis, xaxis));

		glm::mat4 translation  = glm::mat4(1.0f);
		translation[3][0] = -position.x;
		translation[3][1] = -position.y;
		translation[3][2] = -position.z;

		glm::mat4 rotation = glm::mat4(1.0f);
		rotation[0][0] = xaxis.x;
		rotation[1][0] = xaxis.y;
		rotation[2][0] = xaxis.z;
		rotation[0][1] = yaxis.x;
		rotation[1][1] = yaxis.y;
		rotation[2][1] = yaxis.z;
		rotation[0][2] = zaxis.x;
		rotation[1][2] = zaxis.y;
		rotation[2][2] = zaxis.z;

		return rotation * translation;
	}
};

#endif // !CAMERA_H
