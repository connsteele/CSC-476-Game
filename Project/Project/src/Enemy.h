#pragma once

#include "GameObject.h"

#include <iostream>
#include "BaseCode/Shape.h"
#include "BaseCode/Program.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "glm/vec3.hpp"
#include "BaseCode/MatrixStack.h"

// A representation of an enemy unit

class Enemy : public GameObject
{
public:
	Enemy::Enemy(const std::string& name, std::shared_ptr<Shape>& objModel, const std::string& resourceDirectory, std::shared_ptr<Program> curShaderProg, glm::vec3 pos, float vel, glm::vec3 orient, bool visibleBbox, int team);
	bool Enemy::act(std::vector<std::shared_ptr<GameObject> > unitList);

private:
	int distanceLeft;
	bool actionRemaining;
	int attackRange;
	GameObject Enemy::getClosest(std::vector<std::shared_ptr<GameObject> > unitList);
	float Enemy::distance(GameObject unit);
	int Enemy::move(GameObject unit);
	void Enemy::attack(GameObject unit);
};
