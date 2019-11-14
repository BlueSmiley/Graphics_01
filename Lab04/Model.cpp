#include "Model.h"
// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <iostream>
#include <vector>
#include <map>
#include <array>

// Also define for including stb_imae
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Im a big dumb an d this might not work
inline mat4 aiMatrix4x4ToMath(const aiMatrix4x4* from)
{
	mat4 to = mat4(
		(GLfloat)from->a1, (GLfloat)from->a2, (GLfloat)from->a3, (GLfloat)from->a4,
		(GLfloat)from->b1, (GLfloat)from->b2, (GLfloat)from->b3, (GLfloat)from->b4,
		(GLfloat)from->c1, (GLfloat)from->c2, (GLfloat)from->c3, (GLfloat)from->c4,
		(GLfloat)from->d1, (GLfloat)from->d2, (GLfloat)from->d3, (GLfloat)from->d4);
	return to;
}

//inline mat4 aiMatrix4x4ToMath(const aiMatrix4x4* from)
//{
//	mat4 to = mat4(
//		(GLfloat)from->a1, (GLfloat)from->b1, (GLfloat)from->c1, (GLfloat)from->d1,
//		(GLfloat)from->a2, (GLfloat)from->b2, (GLfloat)from->c2, (GLfloat)from->d2,
//		(GLfloat)from->a3, (GLfloat)from->b3, (GLfloat)from->c3, (GLfloat)from->d3,
//		(GLfloat)from->a4, (GLfloat)from->b4, (GLfloat)from->c4, (GLfloat)from->d4);
//	return to;
//}

inline mat3 aiMatrix3x3ToMath(const aiMatrix3x3* from)
{
	mat3 to = mat3(
		(GLfloat)from->a1, (GLfloat)from->a2, (GLfloat)from->a3,
		(GLfloat)from->b1, (GLfloat)from->b2, (GLfloat)from->b3,
		(GLfloat)from->c1, (GLfloat)from->c2, (GLfloat)from->c3);
	return to;
}

//
//inline mat3 aiMatrix3x3ToMath(const aiMatrix3x3* from)
//{
//	mat3 to = mat3(
//		(GLfloat)from->a1, (GLfloat)from->b1, (GLfloat)from->c1,
//		(GLfloat)from->a2, (GLfloat)from->b2, (GLfloat)from->c2,
//		(GLfloat)from->a3, (GLfloat)from->b3, (GLfloat)from->c3);
//	return to;
//}

mat4 convertMat3to4(mat3 org) {
	float* m = org.m;
	mat4 newM = mat4(
		m[0], m[1], m[2], 0.0f,
		m[3], m[4], m[5], 0.0f,
		m[6], m[7], m[8], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	return transpose(newM);
}

void printAssimpMatrix4x4(const aiMatrix4x4* from) {
	printf("%f %f %f %f\n", (GLfloat)from->a1, (GLfloat)from->a2, (GLfloat)from->a3, (GLfloat)from->a4);
	printf("%f %f %f %f\n", (GLfloat)from->b1, (GLfloat)from->b2, (GLfloat)from->b3, (GLfloat)from->b4);
	printf("%f %f %f %f\n", (GLfloat)from->c1, (GLfloat)from->c2, (GLfloat)from->c3, (GLfloat)from->c4);
	printf("%f %f %f %f\n", (GLfloat)from->d1, (GLfloat)from->d2, (GLfloat)from->d3, (GLfloat)from->d4);
}

void printAssimpMatrix3x3(const aiMatrix3x3* from) {
	printf("%f %f %f %f\n", (GLfloat)from->a1, (GLfloat)from->a2, (GLfloat)from->a3);
	printf("%f %f %f %f\n", (GLfloat)from->b1, (GLfloat)from->b2, (GLfloat)from->b3);
	printf("%f %f %f %f\n", (GLfloat)from->c1, (GLfloat)from->c2, (GLfloat)from->c3);
}

// tries to fill 4 most influential bones
VertexBoneData addBoneData(VertexBoneData boneData, unsigned int boneID, float weight) {
	float lowestWeight = -1;
	int lowestWeightIndex = -1;
	for (unsigned int i = 0; i < 4; i++) {
		//checking for an empty slot
		if (boneData.Weights[i] == 0) {
			boneData.Weights[i] = weight;
			boneData.IDs[i] = boneID;
			return boneData;
		}
		else if (lowestWeight < 0 || boneData.Weights[i] < lowestWeight) {
			lowestWeight = boneData.Weights[i];
			lowestWeightIndex = i;
		}
	}
	// No empty slot found so lets evict least found if less than our new bone weight
	if (lowestWeight < weight) {
		boneData.Weights[lowestWeightIndex] = weight;
		boneData.IDs[lowestWeightIndex] = boneID;
	}
	return boneData;
}

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	modelData.scene = aiImportFile(
		file_name,
		aiProcess_Triangulate |aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs  | aiProcess_LimitBoneWeights
	);
	const aiScene* scene = modelData.scene;

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);
	

	modelData.m_GlobalInverseTransform = aiMatrix4x4ToMath(&(scene->mRootNode->mTransformation));
	//printAssimpMatrix4x4(&(scene->mRootNode->mTransformation));
	//printf("Global transform");
	//m_GlobalInverseTransform = rotate_x_deg(m_GlobalInverseTransform, 180);
	//m_GlobalInverseTransform = rotate_y_deg(m_GlobalInverseTransform, 180);
	//m_GlobalInverseTransform = rotate_z_deg(m_GlobalInverseTransform, -90);
	//print(m_GlobalInverseTransform);
	//modelData.m_GlobalInverseTransform = inverse(modelData.m_GlobalInverseTransform);
	//print(m_GlobalInverseTransform);

	unsigned int vertexCount = 0;
	modelData.m_Entries.resize(scene->mNumMeshes);
	

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i bones in mesh\n", mesh->mNumBones);
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			else {
				modelData.mTextureCoords.push_back(vec2());
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
					aiTextureType_DIFFUSE, "texture_diffuse");
				modelData.mTextures.insert(modelData.mTextures.end(), diffuseMaps.begin(), diffuseMaps.end());
				std::vector<Texture> specularMaps = loadMaterialTextures(material,
					aiTextureType_SPECULAR, "texture_specular");
				modelData.mTextures.insert(modelData.mTextures.end(), specularMaps.begin(), specularMaps.end());
			}

		}
	}

	// now adding bones and vertex bone weights
	// Need to do after as any bone can refer to any vertice, even unprocecessed ones maybe
	// So reserve space in advance
	modelData.Bones.resize(modelData.mPointCount);
	unsigned int boneCount = 0;
	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		modelData.m_Entries[m_i] = vertexCount;
		const aiMesh* mesh = scene->mMeshes[m_i];
		vertexCount += mesh->mNumVertices;
		for (unsigned int b_i = 0; b_i < mesh->mNumBones; b_i++) {

			unsigned int boneIndex = 0;
			std::string BoneName(mesh->mBones[b_i]->mName.data);

			if (modelData.m_BoneMapping.find(BoneName) == modelData.m_BoneMapping.end()) {
				// Allocate an index for a new bone
				boneIndex = boneCount;
				modelData.m_BoneMapping[BoneName] = boneIndex;
				boneCount++;
				BoneInfo bi;
				modelData.m_BoneInfo.push_back(bi);
				modelData.m_BoneInfo[boneIndex].inverseBindPoseTransform = aiMatrix4x4ToMath(&(mesh->mBones[b_i]->mOffsetMatrix));
			}
			else {
				boneIndex = modelData.m_BoneMapping[BoneName];
			}

			for (unsigned int j = 0; j < mesh->mBones[b_i]->mNumWeights; j++) {
				unsigned int VertexID = modelData.m_Entries[m_i] + mesh->mBones[b_i]->mWeights[j].mVertexId;
				float boneWeight = mesh->mBones[b_i]->mWeights[j].mWeight;
				VertexBoneData boneData = modelData.Bones[VertexID];
				modelData.Bones[VertexID] = addBoneData(boneData, boneIndex, boneWeight);
				
			}



		}
	}
	modelData.mBoneCount = boneCount;

	for (int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* pAnimation = scene->mAnimations[i];
		std::map<std::string, aiNodeAnim*> nodeMapping;
		for (unsigned int j = 0; j < pAnimation->mNumChannels; j++) {
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[j];

			if (nodeMapping.find(std::string(pNodeAnim->mNodeName.data)) == nodeMapping.end()) {
				nodeMapping[std::string(pNodeAnim->mNodeName.data)] = pNodeAnim;
			}
		}
		modelData.mAnimNodeVector.push_back(nodeMapping);
	}

	//for (int j = 0; j < modelData.mTextureCoords.size(); j++) {
	//	//std::cout << "Loading:" << file_name << std::endl;
	//	print(modelData.mTextureCoords[j]);
	//}

	//uncomment this after I actually migrate code from using assimp structures to own structures
	//aiReleaseImport(scene);
	// just in case any modifications
	modelData.scene = scene;
	return modelData;
}


std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		Texture texture;
		texture.id = TextureFromFile(str.C_Str(),"");
		texture.type = typeName;
		texture.path = str.C_Str();;
		textures.push_back(texture);
	}
	return textures;
}

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
	std::string filename = std::string(path);
	//filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void BoneTransform(ModelData *modelData, float TimeInSeconds, std::vector<mat4>& Transforms)
{
	mat4 identity = identity_mat4();
	if (modelData->scene->HasAnimations()) {
		float TicksPerSecond = modelData->scene->mAnimations[0]->mTicksPerSecond != 0 ?
			modelData->scene->mAnimations[0]->mTicksPerSecond : 24.0f;
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, modelData->scene->mAnimations[0]->mDuration);

		ReadNodeHeirarchy(modelData, AnimationTime, modelData->scene->mRootNode, identity);
	}
	else {
		printf("No animations found \n");
	}

	Transforms.resize(modelData->mBoneCount);

	for (unsigned int i = 0; i < modelData->mBoneCount; i++) {
		Transforms[i] = modelData->m_BoneInfo[i].finalTransform;
		//print((const mat4&)m_BoneInfo[i].finalTransform);
	}

}

void ReadNodeHeirarchy(ModelData *modelData, float AnimationTime, const aiNode* pNode, const mat4& ParentTransform)
{
	std::string nodeName(pNode->mName.data);

	//const aiAnimation* pAnimation = modelData->scene->mAnimations[0];
	std::map<std::string, aiNodeAnim*> nodeAnimMapping = modelData->mAnimNodeVector[0];

	mat4 nodeTransform = aiMatrix4x4ToMath(&(pNode->mTransformation));

	const aiNodeAnim* pNodeAnim = nodeAnimMapping[nodeName];//FindNodeAnimation(pAnimation, nodeName);


	mat4 translation = identity_mat4();
	mat4 scaling = identity_mat4();
	mat4 rotation = identity_mat4();

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		scaling = scale(scaling, vec3(Scaling.x, Scaling.y, Scaling.z));

		//// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		//RotationQ = pNodeAnim->mRotationKeys[0].mValue;
		aiMatrix3x3 rotationM = RotationQ.GetMatrix();
		mat3 tmpRotation = aiMatrix3x3ToMath(&rotationM);
		rotation = convertMat3to4(tmpRotation);
		//print(rotation);


		////// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		//Translation = pNodeAnim->mPositionKeys[0].mValue;
		translation = translate(translation, vec3(Translation.x, Translation.y, Translation.z));


		// Combine the above transformations
	}

	nodeTransform = translation * rotation * scaling;

	//print(nodeTransform);
	//nodeTransform = rotate_x_deg(nodeTransform, 180);
	/*printf("Fixed");
	print(nodeTransform);
	printf("\n");*/

	mat4 globalTransform = ((mat4)ParentTransform) * nodeTransform;
	//print(globalTransform);

	if (modelData->m_BoneMapping.find(nodeName) != modelData->m_BoneMapping.end()) {
		unsigned int boneIndex = modelData->m_BoneMapping[nodeName];

		modelData->m_BoneInfo[boneIndex].finalTransform = modelData->m_GlobalInverseTransform *
			globalTransform * modelData->m_BoneInfo[boneIndex].inverseBindPoseTransform;
		/*printf("Bone Name:%s model space coords:", nodeName.c_str());
		print(globalTransform * modelData->m_BoneInfo[boneIndex].inverseBindPoseTransform);*/
		//m_BoneInfo[boneIndex].finalTransform = globalTransform;
		//print((const mat4&)globalTransform);
		//print((const mat4&)m_BoneInfo[boneIndex].finalTransform);
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(modelData, AnimationTime, pNode->mChildren[i],globalTransform);
	}
}

const aiNodeAnim* FindNodeAnimation(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}


unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}
	printf("Animation time greater than keyframe max");
	return 0;
}


unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (!pNodeAnim->mNumRotationKeys > 0) {
		printf("Error no rotation key frames");
	}

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	printf("Animation time greater than keyframe max");

	return 0;
}


unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (!pNodeAnim->mNumScalingKeys > 0) {
		printf("Error no scaling key frames");
	}

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}
	printf("Animation time greater than keyframe max");
	return 0;
}



void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	//printf("start time%f \n", (float)(pNodeAnim->mPositionKeys[0].mTime));
	unsigned int NextPositionIndex = (PositionIndex + 1);
	//printf("%d\n", pNodeAnim->mNumPositionKeys);
	if (!(NextPositionIndex < pNodeAnim->mNumPositionKeys)) {
		printf("Error:out of position keyframes");
		return;
	}
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	if (!(Factor >= 0.0f && Factor <= 1.0f)) {
		printf("Error:Bad Time info. Animation time was:%d but next frame was at:%d\n", 
				AnimationTime, (float)pNodeAnim->mPositionKeys[PositionIndex].mTime);
		return;
	}
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	if (!(NextRotationIndex < pNodeAnim->mNumRotationKeys)) {
		printf("Error:out of rotation keyframes");
		return;
	}
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	if (!(Factor >= 0.0f && Factor <= 1.0f)) {
		printf("Error:Bad Time info");
		return;
	}
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}


void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	if (!(NextScalingIndex < pNodeAnim->mNumScalingKeys)) {
		printf("Error:out of scaling keyframes");
		Out = aiVector3D(1.0, 1.0, 1.0);
		return;
	}
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	if (!(Factor >= 0.0f && Factor <= 1.0f)) {
		printf("Error:Bad Time info");
		Out = aiVector3D(1.0, 1.0, 1.0);
		return;
	}
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}



char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertexShaderText, const char* fragmentShaderText)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertexShaderText, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentShaderText, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

void generateObjectBufferMesh(Model *model) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	ModelData mesh_data = model->mesh;
	unsigned int vp_vbo = 0;
	unsigned int vn_vbo = 0;
	unsigned int vb_vbo = 0;
	unsigned int vt_vbo = 0;


	GLuint shaderProgramID = model->shaderProgramID;
	GLuint loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	GLuint loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	GLuint loc3 = glGetAttribLocation(shaderProgramID, "tex_coord");
	GLuint loc4 = glGetAttribLocation(shaderProgramID, "bone_ids");
	GLuint loc5 = glGetAttribLocation(shaderProgramID, "bone_weights");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	// debugging code
	/*for (int i = 0;i<mesh_data.Bones.size();i++) {
		VertexBoneData currentBone = mesh_data.Bones[i];
		unsigned int* boneId = currentBone.IDs;
		float* weight = currentBone.Weights;
		for (int j = 0; j < 4; j++) {
			std::cout << "Bone Id:" << boneId[j] << "Weight:" << weight[j] << std::endl;
		}
		std::cout << "Current vertext done" << std::endl;
	}*/
	// debugging code complete

	glGenBuffers(1, &vb_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vb_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh_data.Bones[0]) * mesh_data.Bones.size(), 
		&mesh_data.Bones[0], GL_STATIC_DRAW);

	//	This is for texture coordinates 
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);

	GLuint vao = model->vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);


	glBindBuffer(GL_ARRAY_BUFFER, vb_vbo);
	glEnableVertexAttribArray(loc4);
	glVertexAttribIPointer(loc4, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glEnableVertexAttribArray(loc5);
	glVertexAttribPointer(loc5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);


	model->vao = vao;
}


unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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

void generateSkyBoxBufferMesh(SkyBox *sky) {
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	unsigned int vp_vbo = 0;
	GLuint shaderProgramID = sky->shaderProgramID;
	GLuint loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	sky->mPointCount = sizeof(skyboxVertices) / sizeof(vec3);
	glBufferData(GL_ARRAY_BUFFER,sky->mPointCount*sizeof(vec3), &skyboxVertices, GL_STATIC_DRAW);

	GLuint vao = sky->vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	sky->vao = vao;
}