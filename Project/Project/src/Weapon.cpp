//
// Created by chris on 4/30/19.
//

#include "Weapon.h"

Weapon::Weapon(const std::string &name, std::shared_ptr<Shape> &objModel, const std::string &resourceDirectory,
               std::shared_ptr<Program> curShaderProg, glm::vec3 pos, glm::vec3 orient, bool visibleBbox,
               int team,bool isStaticObject, int weaponType) : GameObject(name, objModel, resourceDirectory, curShaderProg, pos, orient, visibleBbox, team, isStaticObject) {

    //instantiate weapontype
    this->weaponType = weaponType;

}

void Weapon::step(float dt, std::shared_ptr<MatrixStack> &M, std::shared_ptr<MatrixStack> &P, glm::vec3 camLoc, glm::vec3 center, glm::vec3 up)
{
	GameObject::step(dt, M, P, camLoc, center, up);
}
