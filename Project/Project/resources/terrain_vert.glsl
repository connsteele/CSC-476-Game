#version 330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;


// Connor S, stuff for Textures
out vec2 vertex_tex;
out vec3 vertex_nor; // Normals for vertex
out vec3 vertex_pos; // Vertex positions

void main()
{
	gl_Position = P * V * M * vertPos;
	vertex_tex = vertTex;
	vertex_nor = (M * vec4(vertNor, 0.0)).xyz;
	vertex_pos = (M * vertPos).xyz;

	
}
