#version  330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS; //Light space matrix

uniform vec3 lightDir;

/*#version  330 core
layout(location = 0) in vec3 vertPos;*/

out vec2 texCoord;

void main()
{
   gl_Position = vec4(vertPos, 1);
	texCoord = (vertPos.xy+vec2(1, 1))/2.0;
}
