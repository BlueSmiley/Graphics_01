#ifndef _JOINT_HEADER_
#define _JOINT_HEADER_

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include "maths_funcs.h"

struct Joint;

struct Joint {
	GLuint index;
	std::string name;
	std::vector<Joint*> children;
	mat4 animatedTransform;
	mat4 localBindTransform;
	mat4 inverseBindTransform = identity_mat4();
};

// To add children to each Joint
void addChild(Joint *parent,Joint *child);

// Call after setup to calculate for all joints
void calcInverseBindTransforms(mat4 parentBindTransform,Joint *root);

#endif