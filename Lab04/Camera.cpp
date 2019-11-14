#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

void cameraSetup(vec3 cameraOrigin, Camera &camera) {
	camera.pos = cameraOrigin;

	// reverse of the direction the camera is pointing
	vec3 cameraDirection = normalise(camera.pos - camera.front);
	// Up is positive y axis
	camera.right = normalise(cross(camera.up, cameraDirection));

	vec3 cameraUp = cross(cameraDirection, camera.right);
	updateCameraVectors(camera);
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
mat4 GetViewMatrix(Camera &camera)
{
	return look_at(camera.pos, camera.pos + camera.front,camera.cameraUp);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void ProcessKeyboard(Camera_Movement direction, float deltaTime, Camera &camera)
{
	float velocity = camera.speed * deltaTime;
	if (direction == FORWARD)
		camera.pos += camera.front * velocity;
	if (direction == BACKWARD)
		camera.pos -= camera.front * velocity;
	if (direction == LEFT)
		camera.pos -= camera.right * velocity;
	if (direction == RIGHT)
		camera.pos += camera.right * velocity;
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch, Camera &camera)
{
	xoffset *= camera.sensitivity;
	yoffset *= camera.sensitivity;

	camera.yaw += xoffset;
	camera.pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (camera.pitch > 89.0f)
			camera.pitch = 89.0f;
		if (camera.pitch < -89.0f)
			camera.pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors(camera);
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ProcessMouseScroll(float yoffset, Camera &camera)
{
	if (camera.zoom >= 1.0f && camera.zoom <= 45.0f)
		camera.zoom -= yoffset;
	if (camera.zoom <= 1.0f)
		camera.zoom = 1.0f;
	if (camera.zoom >= 45.0f)
		camera.zoom = 45.0f;
}


void updateCameraVectors(Camera &camera) {
	// Calculate the new Front vector
	vec3 front;
	float radYaw = (float)ONE_DEG_IN_RAD * camera.yaw;
	float radPitch = (float)ONE_DEG_IN_RAD * camera.pitch;

	front.v[0] = cosf(radYaw) * cosf(radPitch);
	front.v[1] = sinf(radPitch);
	front.v[2] = sin(radYaw) * cos(radPitch);
	camera.front = normalise(front);
	// Also re-calculate the Right and Up vector
	camera.right = normalise(cross(front, camera.up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera.cameraUp = normalise(cross(camera.right, front));
}