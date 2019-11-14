#ifndef _UTILITY_MYCODE_HEADER_
#define _UTILITY_MYCODE_HEADER_
#include "Model.h"

mat4 calcModeltransform(Model model);
mat4 applyRotations(mat4 model, GLfloat rotations[]);

#endif
