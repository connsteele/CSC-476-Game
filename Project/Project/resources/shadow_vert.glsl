#version  330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
//layout(location = 3) in vec3 vertTan;
//layout(location = 4) in vec3 vertBN;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS; //Light space matrix
uniform vec3 vertTan;
uniform vec3 vertBN;
uniform vec3 lightDir;

out OUT_struct {
	vec3 fPos;
	vec3 fragNor;
	vec2 vTexCoord;
	vec4 fPosLS;
	vec3 vColor;
} out_struct;
out vec3 lightTS;

void main() {
	mat3 TBN;

	vec3 eyeN = (V*M*vec4(vertNor, 0.0)).xyz;
	vec3 eyeT = (V*M*vec4(vertTan, 0.0)).xyz;
	vec3 eyeBN = (V*M*vec4(vertBN, 0.0)).xyz;

	TBN = transpose(mat3(eyeT, eyeBN, eyeN));

  /* First model transforms */
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

	/* the position in world coordinates */
  out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;

	/* the normal */
  out_struct.fragNor = (M*vec4(vertNor, 0.0)).xyz;

  /* pass through the texture coordinates to be interpolated */
  out_struct.vTexCoord = vertTex;

  /* The vertex in light space TODO: fill in appropriately*/
  out_struct.fPosLS = LS * vec4(out_struct.fPos, 1.0);

  lightTS = TBN*normalize(lightDir);
  /* a color that could be blended - or be shading */
  out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightDir)), 0));
}
