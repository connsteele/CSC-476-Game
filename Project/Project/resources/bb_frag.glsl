#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
out vec4 Outcolor;

void main() {
	vec4 texColor0 = texture(Texture0, vTexCoord);

	// IF the alpha value is less than 0.1 discard the color at that location
	if (texColor0.a < 0.1)
	{
		discard;
	}

	Outcolor = texColor0;
}