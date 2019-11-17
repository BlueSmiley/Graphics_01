#include "Collision.h"
#include "Model.h"
#include "Utility.h"
#include <vector>

std::vector<Model*> checkForCollisions(Model moving, std::vector<Model*> background) {
	std::vector<BoundingBox> modelhitbox = moving.hitbox;
	// First calculate new transforms of model
	std::vector<BoundingBox> transformedHitboxes = calculateTransformedHitboxes(moving);
	std::vector<Model*> impacted;

	// Now for each model in background compare against their pre-calculated bounding boxes
	for (int i = 0; i < background.size(); i++) {
		Model compModel = *(background[i]);
		std::vector<BoundingBox> compBoxes = compModel.preTransformedHitboxes;
		bool hit = false;
		// check for collision against any box--usually just one box per model. 
		// But complex models can have multiple, ie. the stage
		for (int j = 0; j < compBoxes.size() && !hit; j++) {
			BoundingBox comparisonHitbox = compBoxes[j];
			// check against the hitboxes of model, sometimes can have multiple usually just one
			for (int k = 0; k < transformedHitboxes.size(); k++) {
				BoundingBox modelBox = transformedHitboxes[k];
				if (checkBoxCollision(modelBox,comparisonHitbox)) {
					hit = true;
					impacted.push_back(background[i]);
					break;
				}
			}
		}
	}
	return impacted;
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
	float minXOne = std::min(one.top_vertex.v[0], one.bottom_vertex.v[0]);
	float maxXOne = std::max(one.top_vertex.v[0], one.bottom_vertex.v[0]);
	float minXTwo = std::min(two.top_vertex.v[0], two.bottom_vertex.v[0]);
	float maxXTwo = std::max(two.top_vertex.v[0], two.bottom_vertex.v[0]);

	float minYOne = std::min(one.top_vertex.v[1], one.bottom_vertex.v[1]);
	float maxYOne = std::max(one.top_vertex.v[1], one.bottom_vertex.v[1]);
	float minYTwo = std::min(two.top_vertex.v[1], two.bottom_vertex.v[1]);
	float maxYTwo = std::max(two.top_vertex.v[1], two.bottom_vertex.v[1]);

	float minZOne = std::min(one.top_vertex.v[2], one.bottom_vertex.v[2]);
	float maxZOne = std::max(one.top_vertex.v[2], one.bottom_vertex.v[2]);
	float minZTwo = std::min(two.top_vertex.v[2], two.bottom_vertex.v[2]);
	float maxZTwo = std::max(two.top_vertex.v[2], two.bottom_vertex.v[2]);

	// Either the plane of the first object is between the second object plane or vice versa
	bool xCheck = 
		(minXOne >= minXTwo && minXOne <= maxXTwo) ||
		(maxXOne >= minXTwo && maxXOne <= maxXTwo) ||
		(minXTwo >= minXOne && minXTwo <= maxXOne) ||
		(maxXTwo >= minXOne && maxXTwo <= maxXOne);
	bool yCheck =
		(minYOne >= minYTwo && minYOne <= maxYTwo) ||
		(maxYOne >= minYTwo && maxYOne <= maxYTwo) ||
		(minYTwo >= minYOne && minYTwo <= maxYOne) ||
		(maxYTwo >= minYOne && maxYTwo <= maxYOne);
	bool zCheck =
		(minZOne >= minZTwo && minZOne <= maxZTwo) ||
		(maxZOne >= minZTwo && maxZOne <= maxZTwo) ||
		(minZTwo >= minZOne && minZTwo <= maxZOne) ||
		(maxZTwo >= minZOne && maxZTwo <= maxZOne);


	if (xCheck && yCheck && zCheck)
		return true;
	return false;
}

