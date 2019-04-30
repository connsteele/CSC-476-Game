//
// Created by chris on 4/30/19.
//
#pragma once


#include "GameObject.h"

class Weapon: public GameObject {
public:
    Weapon(const std::string &name, std::shared_ptr<Shape> &objModel, const std::string &resourceDirectory,
           std::shared_ptr<Program> curShaderProg, glm::vec3 pos, glm::vec3 orient, bool visibleBbox,
           int team);
};
