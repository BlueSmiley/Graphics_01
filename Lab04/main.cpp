// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <limits.h>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>


// Project includes
#include "maths_funcs.h"
#include "Model.h"



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "free3Dmodel.dae"
#define GROUND_MODEL "stage.obj"
#define SWORD_MODEL "Master Sword.obj"
#define SHIELD_MODEL "Zelda Hylian Shield.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

using namespace std;

static const unsigned int MAX_BONES = 200;

Model mainModel;
Model groundModel;
Model swordModel;
Model shieldModel;

int width = 800;
int height = 600;
float deltaTime;
GLfloat rotateSpeed[] = { 10.0f , 10.0f , 10.0f };
GLfloat scalingSpeed[] = { 10.0f, 10.0f, 10.0f } ;
GLfloat translationSpeed[] = { 10.0f, 10.0f, 10.0f };

GLfloat camera_pos[] = { 0.0f,0.0f,0.0f };
GLfloat cameraTranslationSpeed[] = { 100.0f, 100.0f, 100.0f };
GLfloat cameraRotationSpeed[] = { 1.0f, 1.0f, 1.0f };
GLfloat camera_rotations[] = { 0.0f , 0.0f, 0.0f };
GLfloat camera_orbit_rotations[] = { 0.0f , 0.0f, 0.0f };
int prev_mousex = -100;
int prev_mousey = -100;
int mouse_dx = 0;
int mouse_dy = 0;
bool orbit = false;
DWORD animStartTime;
DWORD animRunningTime = 0;

mat4 applyRotations(mat4 model, GLfloat rotations[]) {
	mat4 tmp = model;
	tmp = rotate_x_deg(tmp, rotations[0]);
	tmp = rotate_y_deg(tmp, rotations[1]);
	tmp = rotate_z_deg(tmp, rotations[2]);
	return tmp;
}

mat4 calcModeltransform(Model model) {
	mat4 transform = identity_mat4();
	transform = scale(transform, vec3(model.scale[0], model.scale[1], model.scale[2]));
	transform = applyRotations(transform, model.rotation);
	transform = translate(transform, vec3(model.position[0], model.position[1], model.position[2]));
	return transform;
}

mat4 setupCamera(Model focus) {
	mat4 view = identity_mat4();
	mat4 global = identity_mat4();

	/*vec4 focusCenter = calcModeltransform(focus)*vec4(1.0f, 1.0f, 1.0f, 0.0f);
	view = look_at(vec4(0.0f, 1.0f, -0.0f, 0.0f), focusCenter, vec3(0.0f, 1.0f, 0.0f));*/

	view = applyRotations(view, camera_orbit_rotations);
	view = translate(view, vec3(camera_pos[0], camera_pos[1], camera_pos[2] - 10.0f));
	view = applyRotations(view, camera_rotations);
	return view;
}

mat4 setupPerspective() {
	mat4 persp_proj = perspective(90.0f, (float)width / (float)height, 0.1f, 1000.0f);
	return persp_proj;
}

mat4 renderModelDynamic(Model model, mat4 view, mat4 projection,
	GLfloat parScaling[], GLfloat parRotation[], GLfloat parTranslation[]) {

	ModelData mesh_data = model.mesh;
	GLuint shaderProgramID = model.shaderProgramID;
	glUseProgram(shaderProgramID);
	glBindVertexArray(model.vao);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int bones_mat_location = glGetUniformLocation(shaderProgramID, "bones");


	// Root of the Hierarchy
	mat4 global = identity_mat4();
	mat4 transform = calcModeltransform(model);
	global = scale(global, vec3(parScaling[0], parScaling[1], parScaling[2]));
	global = applyRotations(global,parRotation);
	global = translate(global, vec3(parTranslation[0], parTranslation[1], parTranslation[2]));
	transform = global * transform;

	// calculate bone transforms
	//vector<mat4> boneTransforms;
	//BoneTransform(&(mainModel.mesh), animRunningTime, boneTransforms);
	////printf("%d\n", animRunningTime);
	//for (int tmp = 0; tmp < boneTransforms.size(); tmp++) {
	//	//print(boneTransforms[tmp]);
	//	//printf("Newline\n");
	//}

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, transform.m);


	//GLuint m_boneLocation[MAX_BONES];
	//for (unsigned int i = 0; i < MAX_BONES; i++) {
	//	char Name[128];
	//	memset(Name, 0, sizeof(Name));
	//	snprintf(Name, sizeof(Name), "bones[%d]", i);
	//	m_boneLocation[i] = glGetUniformLocation(shaderProgramID,Name);
	//}
	//for (unsigned int i = 0; i < boneTransforms.size() && i < MAX_BONES; i++) {
	//	glUniformMatrix4fv(m_boneLocation[i], 1, GL_FALSE,boneTransforms[i].m);
	//	//printf("%d\n",boneTransforms[i]);
	//}

	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	return transform;
}

mat4 renderModel(Model model, mat4 view, mat4 projection) {
	ModelData mesh_data = model.mesh;
	GLuint shaderProgramID = model.shaderProgramID;
	glUseProgram(shaderProgramID);
	glBindVertexArray(model.vao);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int bones_mat_location = glGetUniformLocation(shaderProgramID, "bones");


	// Root of the Hierarchy
	mat4 transform = identity_mat4();
	transform = scale(transform, vec3(model.scale[0], model.scale[1], model.scale[2]));
	transform = applyRotations(transform, model.rotation);
	transform = translate(transform, vec3(model.position[0], model.position[1], model.position[2]));

	// calculate bone transforms
	//vector<mat4> boneTransforms;
	//BoneTransform(&(mainModel.mesh), animRunningTime, boneTransforms);
	////printf("%d\n", animRunningTime);
	//for (int tmp = 0; tmp < boneTransforms.size(); tmp++) {
	//	//print(boneTransforms[tmp]);
	//	//printf("Newline\n");
	//}

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, transform.m);


	//GLuint m_boneLocation[MAX_BONES];
	//for (unsigned int i = 0; i < MAX_BONES; i++) {
	//	char Name[128];
	//	memset(Name, 0, sizeof(Name));
	//	snprintf(Name, sizeof(Name), "bones[%d]", i);
	//	m_boneLocation[i] = glGetUniformLocation(shaderProgramID,Name);
	//}
	//for (unsigned int i = 0; i < boneTransforms.size() && i < MAX_BONES; i++) {
	//	glUniformMatrix4fv(m_boneLocation[i], 1, GL_FALSE,boneTransforms[i].m);
	//	//printf("%d\n",boneTransforms[i]);
	//}

	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	return transform;
}

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 view = setupCamera(mainModel);
	mat4 proj = setupPerspective();

	mat4 model = renderModel(mainModel, view, proj);

	//renderModel(groundModel, view, proj);
	renderModelDynamic(swordModel, view, proj, mainModel.scale,mainModel.rotation,mainModel.position);
	renderModelDynamic(shieldModel, view, proj, mainModel.scale, mainModel.rotation, mainModel.position);

	glutSwapBuffers();
}


void updateScene() {

	//Update Delta time
	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	deltaTime = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	animRunningTime = (timeGetTime() - animStartTime)/1000.0f;
	// Rotate the model slowly around the y axis at 20 degrees per second
	//rotate_y += 20.0f * deltaTime;
	//rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders("simpleVertexShader.txt", "simpleFragmentShader.txt");
	mainModel.shaderProgramID = shaderProgramID;
	mainModel.mesh = load_mesh(MESH_NAME);
	// load mesh into a vertex buffer array
	generateObjectBufferMesh(&mainModel);

	
	groundModel.shaderProgramID = shaderProgramID;
	groundModel.mesh = load_mesh(GROUND_MODEL);
	generateObjectBufferMesh(&groundModel);


	swordModel.shaderProgramID = shaderProgramID;
	swordModel.mesh = load_mesh(SWORD_MODEL);
	generateObjectBufferMesh(&swordModel);

	shieldModel.shaderProgramID = shaderProgramID;
	shieldModel.mesh = load_mesh(SHIELD_MODEL);
	generateObjectBufferMesh(&shieldModel);

	animStartTime = timeGetTime();

	mainModel.position[1] -= 5;

	groundModel.scale[0] *= 2;
	groundModel.scale[1] *= 2;
	groundModel.scale[2] *= 2;
	groundModel.position[1] -= 8;

	swordModel.scale[0] *= 0.5;
	swordModel.scale[1] *= 0.5;
	swordModel.scale[2] *= 0.5;
	//swordModel.position[1] -= 3;
	// To move it to roughly the arm length
	swordModel.position[0] -= 2.5;
	// To offset the main model being dragged down to the floor
	swordModel.position[1] += 5;


	shieldModel.scale[0] *= 0.002;
	shieldModel.scale[1] *= 0.002;
	shieldModel.scale[2] *= 0.002;
	// Flip shield to face correct direction
	shieldModel.rotation[1] += 180;
	// Roughly arm length displacement
	shieldModel.position[0] += 2.5;
	// Offset being dropped to floor
	shieldModel.position[1] += 5;

}

#pragma region INPUT_FUNCTIONS
// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == 'q') {
		//Translate the base, etc.
		mainModel.rotation[0] += rotateSpeed[0] * deltaTime;
		mainModel.rotation[0] = fmodf(mainModel.rotation[0], 360.0f);
	}
	if (key == 'w') {
		mainModel.rotation[1] += rotateSpeed[1] * deltaTime;
		mainModel.rotation[1] = fmodf(mainModel.rotation[1], 360.0f);
	}
	if (key == 'e') {
		mainModel.rotation[2] += rotateSpeed[2] * deltaTime;
		mainModel.rotation[2] = fmodf(mainModel.rotation[2], 360.0f);
	}
	if (key == 'a') {
		//Scale in each axis.
		mainModel.scale[0] += scalingSpeed[0] * deltaTime;
	}
	if (key == 's') {
		mainModel.scale[1] += scalingSpeed[1] * deltaTime;
	}
	if (key == 'd') {
		mainModel.scale[2] += scalingSpeed[2] * deltaTime;
	}
	if (key == 'f') {
		mainModel.scale[0] -= scalingSpeed[0] * deltaTime;
		mainModel.scale[1] -= scalingSpeed[1] * deltaTime;
		mainModel.scale[2] -= scalingSpeed[2] * deltaTime;
	}

	if (key == 'z') {
		//translate in each axis.
		mainModel.position[0] += translationSpeed[0] * deltaTime;
	}
	if (key == 'x') {
		mainModel.position[1] += translationSpeed[1] * deltaTime;
	}
	if (key == 'c') {
		mainModel.position[2] += translationSpeed[2] * deltaTime;
	}
	if (key == 'v') {
		//Scale in each axis.
		mainModel.position[0] -= translationSpeed[0] * deltaTime;
	}
	if (key == 'b') {
		mainModel.position[1] -= translationSpeed[1] * deltaTime;
	}
	if (key == 'n') {
		mainModel.position[2] -= translationSpeed[2] * deltaTime;
	}

	if (key == 'o') {
		orbit = !orbit;
	}

	// Draw the next frame
	glutPostRedisplay();

}

void specialKeyboard(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		camera_pos[2] += cameraTranslationSpeed[2] * deltaTime;
		break;
	case GLUT_KEY_DOWN:
		camera_pos[2] -= cameraTranslationSpeed[2] * deltaTime;
		break;
	case GLUT_KEY_LEFT:
		camera_pos[0] -= cameraTranslationSpeed[0] * deltaTime;
		break;
	case GLUT_KEY_RIGHT:
		camera_pos[0] += cameraTranslationSpeed[0] * deltaTime;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouseMove(int x, int y) {
	if(prev_mousex != -100)
		mouse_dx = prev_mousex - x;
	prev_mousex = x;
	if (prev_mousey != -100)
		mouse_dy = prev_mousey - y;
	prev_mousey = y;
	GLfloat* camera_rotations_used;
	if (orbit)
		camera_rotations_used = camera_orbit_rotations;
	else
		camera_rotations_used = camera_rotations;
	camera_rotations_used[1] += mouse_dx*cameraRotationSpeed[1];
	camera_rotations_used[0] += mouse_dy* cameraRotationSpeed[0];
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	// Save the left button state
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN) {
			prev_mousex = x;
			prev_mousey = y;
		}
		else{
			prev_mousex = -100;
			prev_mousey = -100;
		}
	}
}
#pragma endregion INPUT_FUNCTIONS



int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialKeyboard);
	glutMotionFunc(mouseMove);
	glutMouseFunc(mouse);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
