#ifndef _MODEL_HEADER_
#define _MODEL_HEADER_

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <map>
#include "maths_funcs.h"
#include <array>
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

struct Model;
struct ModelData;
struct VertexBoneData;
struct BoneInfo;

struct VertexBoneData
{
	unsigned int IDs[4];
	float Weights[4];
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
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

struct ModelData
{
	std::vector<std::map<std::string, aiNodeAnim*>> mAnimNodeVector;
	const aiScene* scene;
	size_t mPointCount = 0;
	size_t mBoneCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
	std::vector<Texture> mTextures;
	// Lets say each vertex can be affected by at max 4 bones-follwing two == vector<VertexBoneData>
	std::vector<VertexBoneData> Bones;
	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	mat4 m_GlobalInverseTransform;
	std::vector<unsigned int> m_Entries;
	std::vector<BoneInfo> m_BoneInfo;
};

struct BoundingBox {
	// By default initialise to zero
	vec4 top_vertex = vec4(0.0f,0.0f,0.0f,1.0f);
	vec4 bottom_vertex = vec4(0.0f, 0.0f, 0.0f, 1.0f);
};

struct Model {
	ModelData mesh;
	GLuint shaderProgramID;
	GLuint vao;
	GLfloat rotation[3] = { 0.0f,0.0f,0.0f };
	GLfloat scale[3] = { 1.0f,1.0f,1.0f };
	GLfloat position[3] = { 0.0f,0.0f,0.0f };
	std::vector<BoundingBox> hitbox;
	std::vector<BoundingBox> preTransformedHitboxes;
	bool gravityEnabled = false;
};

struct SkyBox {
	unsigned int textureID;
	GLuint shaderProgramID;
	GLuint vao;
	size_t mPointCount = 0;
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
std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

unsigned int loadCubemap(std::vector<std::string> faces);
void generateSkyBoxBufferMesh(SkyBox *sky);

mat4 convertMat3to4(mat3 org);

#endif
