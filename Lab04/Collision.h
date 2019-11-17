#ifndef _COLLISION_HEADER_
#define _COLLISION_HEADER_
#include "Model.h"

//simple rollback based
std::vector<Model*> checkForCollisions(Model moving, std::vector<Model*> background);
bool checkBoxCollision(BoundingBox one, BoundingBox two);
std::vector<BoundingBox> calculateTransformedHitboxes(Model moving);
BoundingBox autoCalculateBoundingBox(Model model);
#endif
