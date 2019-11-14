#include "Collision.h"
#include "Model.h"
#include "Utility.h"
#include <vector>

bool checkForCollisions(Model moving, std::vector<Model*> background) {
	std::vector<BoundingBox> modelhitbox = moving.hitbox;
	// First calculate new transform of model
	std::vector<BoundingBox> transformedHitboxes = calculateTransformedHitboxes(moving);

	// Now for each model in background compare against their pre-calculated bounding boxes
	for (int i = 0; i < background.size(); i++) {
		Model compModel = *(background[i]);
		std::vector<BoundingBox> compBoxes = compModel.preTransformedHitboxes;
		// check for collision against any box--usually just one box per model. 
		// But complex models can have multiple, ie. the stage
		for (int j = 0; j < compBoxes.size(); j++) {
			BoundingBox comparisonHitbox = compBoxes[j];
			// check against the hitboxes of model, sometimes can have multiple usually just one
			for (int k = 0; k < transformedHitboxes.size(); k++) {
				BoundingBox modelBox = transformedHitboxes[k];
				if (checkBoxCollision(modelBox,comparisonHitbox)) {
					return true;
				}
			}
		}
	}
	return false;
}

std::vector<BoundingBox> calculateTransformedHitboxes(Model moving) {
	std::vector<BoundingBox> modelhitbox = moving.hitbox;
	std::vector<BoundingBox> transformedHitboxes;
	// First calculate new transform of model
	for (int i = 0; i < modelhitbox.size(); i++) {
		BoundingBox currentHitbox = modelhitbox[i];
		BoundingBox transformedHitbox;
		transformedHitbox.top_vertex = calcModeltransform(moving)*currentHitbox.top_vertex;
		transformedHitbox.bottom_vertex = calcModeltransform(moving)*currentHitbox.bottom_vertex;

		transformedHitboxes.push_back(transformedHitbox);
	}
	return transformedHitboxes;
}

BoundingBox autoCalculateBoundingBox(Model model) {
	BoundingBox temp;
	for (int i = 0; i < model.mesh.mVertices.size(); i++) {
		for (int j = 0; j < 3; j++) {
			if (model.mesh.mVertices[i].v[j] > temp.top_vertex.v[j]) {
				temp.top_vertex.v[j] = model.mesh.mVertices[i].v[j];
			}
			if (model.mesh.mVertices[i].v[j] < temp.bottom_vertex.v[j]) {
				temp.bottom_vertex.v[j] = model.mesh.mVertices[i].v[j];
			}
		}
	}
	temp.top_vertex.v[3] = 1.0f;
	temp.bottom_vertex.v[3] = 1.0f;
	return temp;
}

bool checkBoxCollision(BoundingBox one, BoundingBox two) {
	bool xCheck = 
		(one.top_vertex.v[0] >= two.top_vertex.v[0] && one.top_vertex.v[0] <= two.bottom_vertex.v[0]) ||
		(one.top_vertex.v[0] <= two.top_vertex.v[0] && one.top_vertex.v[0] >= two.bottom_vertex.v[0]) ||
		(one.bottom_vertex.v[0] >= two.top_vertex.v[0] && one.bottom_vertex.v[0] <= two.bottom_vertex.v[0]) ||
		(one.bottom_vertex.v[0] <= two.top_vertex.v[0] && one.bottom_vertex.v[0] >= two.bottom_vertex.v[0]) ||
		(two.top_vertex.v[0] >= one.top_vertex.v[0] && two.top_vertex.v[0] <= one.bottom_vertex.v[0]) ||
		(two.top_vertex.v[0] <= one.top_vertex.v[0] && two.top_vertex.v[0] >= one.bottom_vertex.v[0]) ||
		(two.bottom_vertex.v[0] >= one.top_vertex.v[0] && two.bottom_vertex.v[0] <= one.bottom_vertex.v[0]) ||
		(two.bottom_vertex.v[0] <= one.top_vertex.v[0] && two.bottom_vertex.v[0] >= one.bottom_vertex.v[0]) ;
	bool yCheck =
		(one.top_vertex.v[1] >= two.top_vertex.v[1] && one.top_vertex.v[1] <= two.bottom_vertex.v[1]) ||
		(one.top_vertex.v[1] <= two.top_vertex.v[1] && one.top_vertex.v[1] >= two.bottom_vertex.v[1]) ||
		(one.bottom_vertex.v[1] >= two.top_vertex.v[1] && one.bottom_vertex.v[1] <= two.bottom_vertex.v[1]) ||
		(one.bottom_vertex.v[1] <= two.top_vertex.v[1] && one.bottom_vertex.v[1] >= two.bottom_vertex.v[1]) ||
		(two.top_vertex.v[1] >= one.top_vertex.v[1] && two.top_vertex.v[1] <= one.bottom_vertex.v[1]) ||
		(two.top_vertex.v[1] <= one.top_vertex.v[1] && two.top_vertex.v[1] >= one.bottom_vertex.v[1]) ||
		(two.bottom_vertex.v[1] >= one.top_vertex.v[1] && two.bottom_vertex.v[1] <= one.bottom_vertex.v[1]) ||
		(two.bottom_vertex.v[1] <= one.top_vertex.v[1] && two.bottom_vertex.v[1] >= one.bottom_vertex.v[1]);
	bool zCheck =
		(one.top_vertex.v[2] >= two.top_vertex.v[2] && one.top_vertex.v[2] <= two.bottom_vertex.v[2]) ||
		(one.top_vertex.v[2] <= two.top_vertex.v[2] && one.top_vertex.v[2] >= two.bottom_vertex.v[2]) ||
		(one.bottom_vertex.v[2] >= two.top_vertex.v[2] && one.bottom_vertex.v[2] <= two.bottom_vertex.v[2]) ||
		(one.bottom_vertex.v[2] <= two.top_vertex.v[2] && one.bottom_vertex.v[2] >= two.bottom_vertex.v[2]) ||
		(two.top_vertex.v[2] >= one.top_vertex.v[2] && two.top_vertex.v[2] <= one.bottom_vertex.v[2]) ||
		(two.top_vertex.v[2] <= one.top_vertex.v[2] && two.top_vertex.v[2] >= one.bottom_vertex.v[2]) ||
		(two.bottom_vertex.v[2] >= one.top_vertex.v[2] && two.bottom_vertex.v[2] <= one.bottom_vertex.v[2]) ||
		(two.bottom_vertex.v[2] <= one.top_vertex.v[2] && two.bottom_vertex.v[2] >= one.bottom_vertex.v[2]);


	if (xCheck && yCheck && zCheck)
		return true;
	return false;
}

