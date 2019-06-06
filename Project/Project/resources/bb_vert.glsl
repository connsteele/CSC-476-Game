#version  330 core
layout(location = 0) in vec2 vertTex; // Connor set this up, removes helps remove errors from the models. Might need to be vec3

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec2 vTexCoord;

void main()
{
	//gl_Position = P * V * M * vertPos;

	vTexCoord = vertTex;
	
}
