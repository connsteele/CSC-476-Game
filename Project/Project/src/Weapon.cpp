//
// Created by chris on 4/30/19.
//

#include "Weapon.h"

Weapon::Weapon(const std::string &name, std::shared_ptr<Shape> &objModel, const std::string &resourceDirectory,
               std::shared_ptr<Program> curShaderProg, glm::vec3 pos, glm::vec3 orient, bool visibleBbox,
               int team, int weaponType) : GameObject(name, objModel, resourceDirectory, curShaderProg, pos, orient, visibleBbox, team) {

    //instantiate weapontype
    this->weaponType = weaponType;

}
