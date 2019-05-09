#include "UIController.h"


//////////////////
///UIController///
//////////////////
void UIController::drawAll() {
	//begin ImGui draw; all draw commands must be performed AFTER here
	ImGui::Begin(name);
	ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGuiWindowFlags_NoBackground;
	ImGuiWindowFlags_NoMove;

	//loop through elements in controller; draw all
	for (int i = 0; i < elements.size(); i++)
		elements[i]->draw();

	//end Imgui draw; all draw commands must be performed BEFORE here
	ImGui::End();
}

/////////////
///UIFrame///
/////////////
void UIFrame::draw() {
}

//////////////
///UIButton///
//////////////
void UIButton::draw() {
	ImGui::Button(name);
}

//////////////
///UISlider///
//////////////
void UISlider::draw() {
	ImGui::SliderFloat(name, &f, min_value, max_value);
}

///////////
///UIBar///
///////////
void UIBar::draw() {
	ImGui::ProgressBar(fraction, ImVec2(sizeX, sizeY), name);
}

////////////////
///UICheckbox///
////////////////
void UICheckbox::draw() {
	ImGui::Checkbox(name, &check);
}

////////////
///UIText///
////////////
void UIText::draw() {
	if (val != -1)
		ImGui::Text(text);

	else
		ImGui::Text(text, val);
}