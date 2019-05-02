#pragma once

// Connor Steele and Chris Gix Game Object Implementation

#include <iostream>
#include "BaseCode/Shape.h"
#include "BaseCode/Program.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "glm/vec3.hpp"
#include "BaseCode/MatrixStack.h"


class GameObject
{
public:
	std::string nameObj;
	std::shared_ptr<Shape> objModel;
	std::shared_ptr<Program> curShaderProg;

	glm::vec3 position, orientation;
	GLuint vbo_vertices;
	GLuint ibo_elements;

	glm::vec3 bboxSize;
	glm::vec3 bboxCenter;
	glm::mat4 bboxTransform;
	bool hitByPlayer, visibleBbox, isPosessed, isRender, beenShot;
	int team; // 0 is neutral, 1 for robots(player team), 2 for aliens
	int currWeapon; //0 for default weapon, 1 for sphere gun, 2 for shotgun

	GLfloat min_x, max_x,
		min_y, max_y,
		min_z, max_z;



	GameObject(const std::string& name, std::shared_ptr<Shape>& objModel, const std::string& resourceDirectory, std::shared_ptr<Program> curShaderProg, glm::vec3 pos, glm::vec3 orient, bool visibleBbox, int team);
	void DrawGameObj();
	void renderBbox();
	void initBbox();
	void step(float dt, std::shared_ptr<MatrixStack> &M, std::shared_ptr<MatrixStack> &P, glm::vec3 camLoc, glm::vec3 center, glm::vec3 up);
	void DoCollisions(std::shared_ptr<MatrixStack> &M); //std::shared_ptr<GameObject> world
	~GameObject(); // Destroyer

private:
	float elapsedTime = 0.0f;
	

};

