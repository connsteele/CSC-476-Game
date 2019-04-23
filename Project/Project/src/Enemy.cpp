#include "Enemy.h"

// A representation of an enemy unit

Enemy::Enemy(const std::string& gameObjName, std::shared_ptr<Shape>& objModel, const std::string& resourceDirectory, std::shared_ptr<Program> curShaderProg, glm::vec3 pos, float vel, glm::vec3 orient, bool visibleBbox, int team)
	: GameObject(gameObjName,  objModel,  resourceDirectory,  curShaderProg,  pos, vel, orient, visibleBbox, team)
{
	distanceLeft = 20;
	attackRange = 20;
	actionRemaining = TRUE;
}

bool Enemy::act(std::vector<std::shared_ptr<GameObject> > unitList)
{
	if (actionRemaining)
	{
		GameObject closest = getClosest(unitList);
		if (distance(closest) < attackRange)
		{
			attack(closest);
		}
		else
		{
			move(closest);
		}
	}

	return actionRemaining;
}

GameObject Enemy::getClosest(std::vector<std::shared_ptr<GameObject> > unitList)
{
	GameObject closest = *unitList[0];
	float min = FLT_MAX;
	float cur = 0.0f;
	for (size_t i = 0; i < unitList.size(); i++)
	{
		//test for player unit
		if ((*unitList[i]).team == 1)
		{
			cur = distance(*unitList[i]);
			if (min > cur)
			{
				min = cur;
				closest = *unitList[i];
			}
		}
	}
	return closest;
}

float Enemy::distance(GameObject unit)
{
	float diff_x = position.x - unit.position.x;
	float diff_z = position.z - unit.position.z;
	return sqrt(diff_x*diff_x + diff_z*diff_z);
}

int Enemy::move(GameObject unit)
{
	//calc vect direction
	float x = unit.position.x - position.x;
	float z = unit.position.z - position.z;

	//scale to 5 units
	float scalar = 5 / sqrt(x*x + z * z);
	x = scalar * x;
	z = scalar * z;
	//move position
	position.x = position.x + x;
	position.z = position.z + z;

	distanceLeft = distanceLeft - 5;
	if (distanceLeft == 0)
	{
		actionRemaining = false;
	}
	return distanceLeft;
}

void Enemy::attack(GameObject unit)
{
	printf("%s attacked %s\n", nameObj.c_str(), unit.nameObj.c_str());

	actionRemaining = false;
}
