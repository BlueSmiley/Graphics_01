#ifndef _CAMERA_HEADER_
#define _CAMERA_HEADER_



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};


// WIP Should be used for first person camera and then strip out zoom
#include "maths_funcs.h"
//FPS free form camera based on learnOpenGl tutorial
struct Camera {
	// by default start camera at origin 
	vec3 pos = vec3(0.0f,0.0f,0.0f);
	// have it looking at somethung in front of it, default -z
	vec3 front = vec3(0.0f, 0.0f, -1.0f);
	// by default use positive y as up
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right;
	// by default leave the camera up as the world up
	vec3 cameraUp = up;
	float yaw = -90.0f;
	float pitch = 0.0f;
	float speed = 10.0f;
	float sensitivity = 0.1f;
	float zoom = 100.0f;

};

mat4 GetViewMatrix(Camera &camera);
void cameraSetup(vec3 cameraOrigin,Camera &camera);
void updateCameraVectors(Camera &camera);
void ProcessKeyboard(Camera_Movement direction, float deltaTime, Camera &camera);
void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch, Camera &camera);
void ProcessMouseScroll(float yoffset, Camera &camera);
#endif
