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
#include "Utility.h"
#include "Collision.h"
#include "Camera.h"



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "fml.dae"
#define GROUND_MODEL "stage2.obj"
#define SWORD_MODEL "Master Sword.obj"
#define SHIELD_MODEL "Zelda Hylian Shield.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

using namespace std;

static const unsigned int MAX_BONES = 200;

struct Animator {
	bool start = false;
	float elapsed = 0.0f;
	GLfloat preRotation[3];
	GLfloat preTranslation[3];
	GLfloat preScaling[3];
	float playTime;
};

SkyBox sky;
Model mainModel;
Model groundModel;
Model swordModel;
Model shieldModel;
Model collisionCheck;

// A far better way would be to have one vector that stores all models
// have the indexs of the relevant models as #defines and
// just have bools on each model that indicate these traits
// but I don't have time to refactor significantly rn, since i'm trying to get as many features done as possible
std::vector<Model*> collidables;
std::vector<Model*> gravityEnabledModels;

int width = 800;
int height = 600;
float deltaTime;
float gravityStrength = 1.0f;
GLfloat rotateSpeed[] = { 10.0f , 10.0f , 10.0f };
GLfloat scalingSpeed[] = { 10.0f, 10.0f, 10.0f } ;
GLfloat translationSpeed[] = { 5.0f, 5.0f, 5.0f };

Camera camera;
int prev_mousex = -100;
int prev_mousey = -100;
int mouse_dx = 0;
int mouse_dy = 0;
bool orbit = true;
DWORD animStartTime;
DWORD animRunningTime = 0;
Animator block;
Animator attack;

int sign(float num) {
	if (num < 0) {
		return -1;
	}
	else {
		return 1;
	}
}

void copyFloatArrays(GLfloat mat1[3], GLfloat mat2[3]) {
	for (int i = 0; i < 3; i++) {
		mat1[i] = mat2[i];
	}
}

mat4 setupCamera(Model focus) {
	//mat4 view = identity_mat4();

	mat4 focusTransform = calcModeltransform(focus);
	mat4 view = GetViewMatrix(camera);
	return view;
}

mat4 setupPerspective() {
	mat4 persp_proj = perspective(75.0f, (float)width / (float)height, 0.1f, 1000.0f);
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

	int view_pos_location = glGetUniformLocation(shaderProgramID, "viewPos");
	vec3 cameraPos = camera.pos;
	glUniform3fv(view_pos_location, sizeof(cameraPos.v), cameraPos.v);


	// Root of the Hierarchy
	mat4 global = identity_mat4();
	mat4 transform = calcModeltransform(model);
	global = scale(global, vec3(parScaling[0], parScaling[1], parScaling[2]));
	global = applyRotations(global,parRotation);
	global = translate(global, vec3(parTranslation[0], parTranslation[1], parTranslation[2]));
	transform = global * transform;

	// calculate bone transforms
	vector<mat4> boneTransforms;
	// 10 is anim running time
	BoneTransform(&(mainModel.mesh), animRunningTime, boneTransforms);
	//printf("%d\n", animRunningTime);
	/*for (int tmp = 0; tmp < boneTransforms.size(); tmp++) {
		print(boneTransforms[tmp]);
		printf("\n");
	}*/

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, transform.m);


	GLuint m_boneLocation[MAX_BONES];
	for (unsigned int i = 0; i < MAX_BONES; i++) {
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "bones[%d]", i);
		m_boneLocation[i] = glGetUniformLocation(shaderProgramID,Name);
	}
	for (unsigned int i = 0; i < boneTransforms.size() && i < MAX_BONES; i++) {
		glUniformMatrix4fv(m_boneLocation[i], 1, GL_FALSE,boneTransforms[i].m);
		//printf("%d\n",boneTransforms[i]);
	}

	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	return transform;
}


void renderSkyBox(SkyBox sky, mat4 view, mat4 projection) {
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	GLuint shaderProgramID = sky.shaderProgramID;

	glUseProgram(shaderProgramID);
	glBindVertexArray(sky.vao);
	//printf("Vao %d Shader_ID %d Texture_ID %d \n", sky.vao,sky.shaderProgramID, sky.textureID);
	glActiveTexture(GL_TEXTURE0);
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int texture_location = glGetUniformLocation(shaderProgramID, "skybox");
	mat4 view_untranslated = mat4(
			view.m[0], view.m[4], view.m[8], 0.0f,
			view.m[1], view.m[5], view.m[9], 0.0f,
			view.m[2], view.m[6], view.m[10], 0.0f,
			0.0f, 0.0f, 0.0f, view.m[15]);
	/*print(view);
	print(view_untranslated);
	printf("fuck me\n");*/
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_untranslated.m);
	glUniform1i(texture_location, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sky.textureID);
	
	glDrawArrays(GL_TRIANGLES, 0, sky.mPointCount);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

mat4 renderModel(Model model, mat4 view, mat4 projection) {
	ModelData mesh_data = model.mesh;
	GLuint shaderProgramID = model.shaderProgramID;
	glUseProgram(shaderProgramID);
	glBindVertexArray(model.vao);

	// Load a new texture and replace the material textures like a god :)
	std::vector<Texture> textures;
	Texture texture;
	texture.id = TextureFromFile("steve.png", "");
	texture.type = "texture_diffuse";
	texture.path = "steve.png";
	textures.push_back(texture);
	mesh_data.mTextures = textures;

	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	for (unsigned int i = 0; i < mesh_data.mTextures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		string number;
		string name = mesh_data.mTextures[i].type;

		if (name == "texture_diffuse" && diffuseNr<=3)
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular" && specularNr <= 3)
			number = std::to_string(specularNr++);

		int texture_location = glGetUniformLocation(shaderProgramID, (name + number).c_str());
		glUniform1f(texture_location, i);
		glBindTexture(GL_TEXTURE_2D, mesh_data.mTextures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int bones_mat_location = glGetUniformLocation(shaderProgramID, "bones");

	int view_pos_location = glGetUniformLocation(shaderProgramID, "viewPos");
	vec3 cameraPos = camera.pos;
	//print(cameraPos);
	glUniform3fv(view_pos_location, sizeof(cameraPos.v),cameraPos.v);


	// Root of the Hierarchy
	mat4 transform = identity_mat4();
	transform = scale(transform, vec3(model.scale[0], model.scale[1], model.scale[2]));
	transform = applyRotations(transform, model.rotation);
	transform = translate(transform, vec3(model.position[0], model.position[1], model.position[2]));

	// calculate bone transforms
	vector<mat4> boneTransforms;
	BoneTransform(&(mainModel.mesh), animRunningTime, boneTransforms);
	//printf("%d\n", animRunningTime);
	// debug code
	/*for (int tmp = 0; tmp < boneTransforms.size(); tmp++) {
		print(boneTransforms[tmp]);
		printf("\n");
	}
	printf("End of bonetransforms,size: %d\n", boneTransforms.size());*/
	//printf("Actual num of bones:%d\n\n", mainModel.mesh.mBoneCount);
	// end debug code

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, transform.m);


	GLuint m_boneLocation[MAX_BONES];
	for (unsigned int i = 0; i < MAX_BONES; i++) {
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "bones[%d]", i);
		m_boneLocation[i] = glGetUniformLocation(shaderProgramID,Name);
	}
	for (unsigned int i = 0; i < boneTransforms.size() && i < MAX_BONES; i++) {
		glUniformMatrix4fv(m_boneLocation[i], 1, GL_FALSE,boneTransforms[i].m);
		//printf("%d\n",boneTransforms[i]);
	}

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

	renderSkyBox(sky, view, proj);
	mat4 model = renderModel(mainModel, view, proj);
	renderModel(collisionCheck, view, proj);

	renderModel(groundModel, view, proj);
	renderModelDynamic(swordModel, view, proj, mainModel.scale,mainModel.rotation,mainModel.position);
	renderModelDynamic(shieldModel, view, proj, mainModel.scale, mainModel.rotation, mainModel.position);

	glutSwapBuffers();
}

void playBlockAnim(Model *model) {
	model->rotation[2] += 50.0f;
}

void playAttackAnim(Model *model) {
	model->rotation[0] += 50.0f;
}

void playAnims() {
	if (block.start) {
		playBlockAnim(&shieldModel);
		block.elapsed += deltaTime;
		if (block.elapsed > block.playTime) {
			// Copy back array data to pre-animation transforms
			copyFloatArrays(shieldModel.rotation, block.preRotation);
			copyFloatArrays(shieldModel.position, block.preTranslation);
			copyFloatArrays(shieldModel.scale, block.preScaling);
			block.start = false;
			block.elapsed = 0.0f;
		}
	}

	if (attack.start) {
		playAttackAnim(&swordModel);
		attack.elapsed += deltaTime;
		if (attack.elapsed > attack.playTime) {
			// Copy back array data to pre-animation transforms
			copyFloatArrays(swordModel.rotation, attack.preRotation);
			copyFloatArrays(swordModel.position, attack.preTranslation);
			copyFloatArrays(swordModel.scale, attack.preScaling);
			attack.start = false;
			attack.elapsed = 0.0f;
		}
	}
}

// move physics calculation to own cpp and header
bool grounded(Model model) {
	std::vector<vec3> vertices = model.mesh.mVertices;
	mat4 transform = calcModeltransform(model);
	for (vec3 vertex : vertices) {
		vec4 transformedVertice = transform * vec4(vertex.v[0], vertex.v[1], vertex.v[2], 1.0f);
		if (transformedVertice.v[1] <= 0.0f)
			return true;
	}
	return false;
}

void applyGravity(std::vector<Model*> models, float deltaTime) {
	for (int i = 0; i < models.size(); i++) {
		if (!grounded(*models[i])) {
			models[i]->position[1] -= gravityStrength*deltaTime;
		}
	}
}

void updateScene() {

	//Update Delta time
	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	deltaTime = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	playAnims();
	animRunningTime = (timeGetTime() - animStartTime)/1000.0f;

	applyGravity(gravityEnabledModels, deltaTime);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{

	attack.playTime = 2;
	block.playTime = 2;

	// Set up the shaders
	GLuint shaderProgramID = CompileShaders("simpleVertexShader.txt", "simpleFragmentShader.txt");
	mainModel.shaderProgramID = shaderProgramID;
	mainModel.mesh = load_mesh(MESH_NAME);
	// load mesh into a vertex buffer array
	generateObjectBufferMesh(&mainModel);
	// Create hitboxes for model
	std::vector<BoundingBox> hitboxes;
	hitboxes.push_back(autoCalculateBoundingBox(mainModel));
	mainModel.hitbox = hitboxes;
	mainModel.preTransformedHitboxes = hitboxes;
	gravityEnabledModels.push_back(&mainModel);

	GLuint staticShaderProgramID = CompileShaders("StaticVertexShader.txt", "simpleFragmentShader.txt");
	collisionCheck.shaderProgramID = staticShaderProgramID;
	collisionCheck.mesh = load_mesh(MESH_NAME);
	generateObjectBufferMesh(&collisionCheck);
	hitboxes.clear();
	hitboxes.push_back(autoCalculateBoundingBox(collisionCheck));
	collisionCheck.hitbox = hitboxes;
	collisionCheck.position[0] += 10;
	collisionCheck.position[1] -= 5;
	collisionCheck.preTransformedHitboxes = calculateTransformedHitboxes(collisionCheck);
	collidables.push_back(&collisionCheck);


	groundModel.shaderProgramID = staticShaderProgramID;
	groundModel.mesh = load_mesh(GROUND_MODEL);
	generateObjectBufferMesh(&groundModel);

	
	swordModel.shaderProgramID = staticShaderProgramID;
	swordModel.mesh = load_mesh(SWORD_MODEL);
	generateObjectBufferMesh(&swordModel);

	shieldModel.shaderProgramID = staticShaderProgramID;
	shieldModel.mesh = load_mesh(SHIELD_MODEL);
	generateObjectBufferMesh(&shieldModel);

	animStartTime = timeGetTime();

	//mainModel.position[1] -= 5;

	groundModel.scale[0] *= 3;
	groundModel.scale[1] *= 3;
	groundModel.scale[2] *= 3;
	groundModel.position[1] -= 10;

	swordModel.scale[0] *= 0.5;
	swordModel.scale[1] *= 0.5;
	swordModel.scale[2] *= 0.5;
	//swordModel.position[1] -= 3;
	// To move it to roughly the arm length
	swordModel.position[2] -= 2.5;
	// To offset the main model being dragged down to the floor
	//swordModel.position[1] += 5;


	shieldModel.scale[0] *= 0.002;
	shieldModel.scale[1] *= 0.002;
	shieldModel.scale[2] *= 0.002;
	// Flip shield to face correct direction
	shieldModel.rotation[1] += 180;
	// Roughly arm length displacement
	shieldModel.position[2] += 5;
	// Offset being dropped to floor
	//shieldModel.position[1] += 5;

	cameraSetup(vec3(0.0f, 0.0f, 0.0f), camera);

	// Load skybox
	vector<std::string> faces
	{
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	sky.textureID = cubemapTexture;
	GLuint skyShaderId = CompileShaders("SkyBoxVShader.txt", "SkyBoxFShader.txt");
	sky.shaderProgramID = skyShaderId;
	generateSkyBoxBufferMesh(&sky);
}

void playAnim(Animator &block, Model &model) {
	// if animation is already playing don't replace transforms
	if (!(block.start)) {
		copyFloatArrays(block.preRotation, model.rotation);
		copyFloatArrays(block.preTranslation, model.position);
		copyFloatArrays(block.preScaling, model.scale);
	}
	block.start = true;
	block.elapsed = 0.0f;
}

#pragma region INPUT_FUNCTIONS
// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {

	GLfloat tempPos[3] = { mainModel.position[0], mainModel.position[1] ,mainModel.position[2] };
	if (key == 'w') {
		mainModel.position[2] += translationSpeed[2] * deltaTime;
		// face forward
		mainModel.rotation[1] = 0.0f;
	}
	if (key == 's') {
		mainModel.position[2] -= translationSpeed[2] * deltaTime;
		// face backwards
		mainModel.rotation[1] = 180.0f;
	}
	if (key == 'a') {
		mainModel.position[0] += translationSpeed[0] * deltaTime;
		// Face left
		mainModel.rotation[1] = 90.0f;
	}
	if (key == 'd') {
		mainModel.position[0] -= translationSpeed[0] * deltaTime;
		// Face Right
		mainModel.rotation[1] = 270.0f;
	}

	if (key == 'b') {
		playAnim(block,shieldModel);
	}
	if (key == 'n') {
		playAnim(attack, swordModel);
	}

	if (key == 'o') {
		orbit = !orbit;
	}
	if (key == 'j') {
		mainModel.position[1] += 10.0f;
	}

	if (checkForCollisions(mainModel,collidables)) {
		printf("You hit me :(\n");
		mainModel.position[0] = tempPos[0];
		mainModel.position[1] = tempPos[1];
		mainModel.position[2] = tempPos[2];
	}

	// Draw the next frame
	glutPostRedisplay();

}

void specialKeyboard(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		//camera_pos[2] -= cameraTranslationSpeed[2] * deltaTime;
		ProcessKeyboard(FORWARD, deltaTime, camera);
		break;
	case GLUT_KEY_DOWN:
		ProcessKeyboard(BACKWARD, deltaTime, camera);
		break;
	case GLUT_KEY_LEFT:
		ProcessKeyboard(LEFT, deltaTime, camera);
		break;
	case GLUT_KEY_RIGHT:
		ProcessKeyboard(RIGHT, deltaTime, camera);
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

	ProcessMouseMovement(-mouse_dx, mouse_dy, true, camera);
	/*GLfloat* camera_rotations_used;
	if (orbit)
		camera_rotations_used = camera_orbit_rotations;
	else
		camera_rotations_used = camera_rotations;
	camera_rotations_used[1] -= mouse_dx*cameraRotationSpeed[1] * deltaTime;;
	camera_rotations_used[0] += mouse_dy* cameraRotationSpeed[0] * deltaTime;;*/
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
