#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 2) in vec3 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fragPos;

void main()
{
	mat4 ViewRotation = mat4(mat3(V));
	gl_Position = P * ViewRotation * M * vertPos;
	fragPos = vec3(vertPos);
	gl_Position = gl_Position.xyww;
}
