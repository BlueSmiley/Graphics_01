#include "Joint.h"
#include <vector>

void addChild(Joint *parent, Joint *child) {
	std::vector<Joint*> children = parent->children;
	children.push_back(child);
}

void calcInverseBindTransforms(mat4 parentBindTransform, Joint *root) {
	mat4 localBindTransform = root->localBindTransform;
	mat4 bindTransform = parentBindTransform * (root->localBindTransform);
	root->inverseBindTransform = inverse(bindTransform);
	std::vector<Joint*> children = root->children;
	for (Joint *child : children) {
		calcInverseBindTransforms(bindTransform, child);
	}
}