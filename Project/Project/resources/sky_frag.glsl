#version 330 core 
//to send the color to a frame buffer
out vec4 color;

in vec3 fragPos;

uniform samplerCube gCubemapTexture;
void main()
{
	vec4 fragColor;
	fragColor = texture(gCubemapTexture, fragPos);
	color = fragColor;
}