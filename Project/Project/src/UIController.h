#pragma once

#include <iostream>
#include <vector>
#include "imgui-master/imgui.h"
#include "imgui-master/examples/imgui_impl_opengl3.h"
#include "imgui-master/examples/imgui_impl_glfw.h"
#include "glm/glm.hpp"

class UIElement {
protected:
	glm::vec3 pos, mainColor;
	int ID;

public:
	UIElement() {}
	UIElement(glm::vec3 pos, glm::vec3 mainColor, int ID) {
		this->pos = pos;
		this->mainColor = mainColor;
		this->ID = ID;
	}
	virtual void draw() = 0;
};

class UIFrame : public UIElement {
private:
	std::vector<UIElement*> elements;
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

	void addElement(UIElement* newUI) { elements.push_back(newUI); }
	void draw();
};

class UIButton : public UIElement {
private:
	char* name;
	glm::vec3 pressedColor;

public:
	UIButton(glm::vec3 pos, glm::vec3 color, int ID, char* name) {
		this->pos = pos;
		this->mainColor = mainColor;
		this->ID = ID;
		this->pos = pos;
		this->name = name;
	}

	void draw();
};

class UISlider : public UIElement {
private:
	float f, min_value, max_value, power, init_value;
	char* name;

public:
	UISlider(char* name, glm::vec3 pos, glm::vec3 color, int ID, float min_value, float max_value, float power, float init_value) {
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

class UIBar : public UIElement {
private:
	float fraction, sizeX, sizeY;
	char* name;

public:
	UIBar(glm::vec3 pos, glm::vec3 color, int ID, float fraction, char* name, float sizeX, float sizeY) {
		this->pos = pos;
		this->mainColor = color;
		this->ID = ID;
		this->fraction = fraction;
		this->sizeX = sizeX;
		this->sizeY = sizeY;
		this->name = name;
	}

	void draw();
};

class UICheckbox : public UIElement {
private:
	char* name;
	bool check = false;

public:
	UICheckbox(glm::vec3 pos, glm::vec3 color, int ID, char* name) {
		this->pos = pos;
		this->mainColor = color;
		this->ID = ID;
		this->name = name;
	}

	void draw();
	bool getCheck() { return check; }
 };

class UIText : public UIElement {
private:
	char* text;
	float val = -1;

public:
	UIText(char* text) { this->text = text; }
	UIText(char* text, float val) { 
		this->text = text; 
		this->val = val;
	}
	void draw();
};

class UIController {
private:
	std::vector<UIElement*> elements;
	float sizeX, sizeY;
	bool render = false;
	char* name;

public:
	UIController() {}
	void addElement(UIElement* newUI) { elements.push_back(newUI); }
	void drawAll();
	void setRender(bool r) { render = r; }
	bool shouldRender() { return render; }
	void setName(char* name) { this->name = name; }
	void setSize(float sizeX, float sizeY) { this->sizeX = sizeX; this->sizeY = sizeY; }
};

