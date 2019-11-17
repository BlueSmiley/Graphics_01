#include "Utility.h"

mat4 calcModeltransform(Model model) {
	mat4 transform = identity_mat4();
	transform = scale(transform, vec3(model.scale[0], model.scale[1], model.scale[2]));
	transform = applyRotations(transform, model.rotation);
	transform = translate(transform, vec3(model.position[0], model.position[1], model.position[2]));
	return transform;
}

mat4 applyRotations(mat4 model, GLfloat rotations[]) {
	mat4 tmp = model;
	tmp = rotate_x_deg(tmp, rotations[0]);
	tmp = rotate_y_deg(tmp, rotations[1]);
	tmp = rotate_z_deg(tmp, rotations[2]);
	return tmp;
}