#ifndef _MODEL_HEADER_
#define _MODEL_HEADER_

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include "maths_funcs.h"
#include <array>
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

struct Model;
struct ModelData;
struct VertexBoneData;

struct VertexBoneData
{
	unsigned int IDs[4];
	float Weights[4];
};

struct ModelData
{
	const aiScene* scene;
	size_t mPointCount = 0;
	size_t mBoneCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
	// Lets say each vertex can be affected by at max 4 bones-follwing two == vector<VertexBoneData>
	std::vector<VertexBoneData> Bones;
};


struct BoneInfo {
	mat4 inverseBindPoseTransform;
	mat4 finalTransform;

	BoneInfo()
	{
		inverseBindPoseTransform = zero_mat4();
		finalTransform = zero_mat4();
	}
};

struct Model {
	ModelData mesh;
	GLuint shaderProgramID;
	GLuint vao;
	GLfloat rotation[3] = { 0.0f,0.0f,0.0f };
	GLfloat scale[3] = { 1.0f,1.0f,1.0f };
	GLfloat position[3] = { 0.0f,0.0f,0.0f };
};

ModelData load_mesh(const char* file_name);

char* readShaderSource(const char* shaderFile);
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
GLuint CompileShaders(const char* vertexShaderText, const char* fragmentShaderText);

void generateObjectBufferMesh(Model *model);
const aiNodeAnim* FindNodeAnimation(const aiAnimation* pAnimation, const std::string NodeName);
void BoneTransform(ModelData *modelData, float TimeInSeconds, std::vector<mat4>& Transforms);
void ReadNodeHeirarchy(ModelData *modelData, float AnimationTime, const aiNode* pNode, const mat4& ParentTransform);
VertexBoneData addBoneData(VertexBoneData boneData, unsigned int boneID, float weight);

void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);


#endif
