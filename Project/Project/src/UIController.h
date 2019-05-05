#pragma once

#include <iostream>
#include <vector>
#include "imgui-master/imgui.h"
#include "glm/glm.hpp"

class UIController {
protected:
	glm::vec3 pos, mainColor;
	int ID;

public:
	UIController() {}
	UIController(glm::vec3 pos, glm::vec3 mainColor, int ID) {
		this->pos = pos;
		this->mainColor = mainColor;
		this->ID = ID;
	}
};

class UIFrame : public UIController {
private:
	std::vector<UIController> elements;
	float width, height;
	int pos_flags, window_flags, size_flags;

public:
	UIFrame(glm::vec3 pos, glm::vec3 mainColor, int ID, float width, float height, int pos_flags, int window_flags, int size_flags) {
		this->pos = pos;
		this->mainColor = mainColor;
		this->ID = ID;
		this->width = width;
		this->height = height;
		this->pos_flags = pos_flags;
		this->window_flags = window_flags;
		this->size_flags = size_flags;
	}

	void addElement(UIController newUI) { elements.push_back(newUI); }
	void draw();
};

class UIButton : public UIController {
private:
	std::string name;
	glm::vec3 pressedColor;

public:
	UIButton(glm::vec3 pos, glm::vec3 color, int ID, std::string name) {
		this->pos = pos;
		this->mainColor = mainColor;
		this->ID = ID;
		this->pos = pos;
		this->name = name;
	}

	void draw();
};

class UISlider : public UIController {
private:
	float min_value, max_value, power, init_value;

public:
	UISlider(glm::vec3 pos, glm::vec3 color, int ID, float min_value, float max_value, float power, float init_value) {
		this->pos = pos;
		this->mainColor = color;
		this->ID = ID;
		this->min_value = min_value;
		this->max_value = max_value;
		this->power = power;
		this->init_value = init_value;
	}

	void draw();
};

class UIBar : public UIController {
private:
	float fraction;
	std::string name;

public:
	UIBar(glm::vec3 pos, glm::vec3 color, int ID, float fraction, std::string name) {
		this->pos = pos;
		this->mainColor = color;
		this->ID = ID;
		this->fraction = fraction;
		this->name = name;
	}

	void draw();
};

