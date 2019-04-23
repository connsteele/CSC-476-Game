#pragma once
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class GUIElement {
private:
	vec3 pos;
	string imageFilePath;
	//render program

public:
	GUIElement() {}
	GUIElement(vec3 position, string imageName) {
		pos = position;
		imageFilePath = imageName;
	}
	void drawGUIElement();
	void updatePos(vec3 newPos) { pos = newPos; }
	void updateImage(string imageName) { imageFilePath = imageName; }
	vec3 getPos() { return pos; }
	void draw() {}
};

/*class GUIButton : GUIElement {
private:
	string pressedImagePath;
public:
};*/

class GUIController {
private:
	vector<GUIElement> GUI_objs;

public:
	GUIController() {}
	void drawCanvas() {
		int numElements = GUI_objs.size();
		for (int i = 0; i < numElements; i++)
			GUI_objs[i].draw();
	}
	void printElements();
	void addElement(GUIElement newGUI) { GUI_objs.push_back(newGUI); }
	void removeElement(int index) { GUI_objs.erase(GUI_objs.begin()+(index-1)); }
	GUIElement getElement(int index) { return GUI_objs[index]; }
	int getSize() { return GUI_objs.size(); }
	bool isEmpty() { return GUI_objs.empty(); }
};