#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec2 vertTex;

out vec2 vTexCoord;

void main()
{
	gl_Position = vertPos;

    vTexCoord = (vertPos.xy+vec2(1, -1)) / 2.0;

}
