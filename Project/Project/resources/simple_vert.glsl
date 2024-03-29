#version  330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex; // Connor set this up, removes helps remove errors from the models. Might need to be vec3

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 fragNor;
out vec3 WPos;

void main()
{
	gl_Position = P * V * M * vertPos;

   
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
   
	WPos = (M * vertPos).xyz;
	
}
