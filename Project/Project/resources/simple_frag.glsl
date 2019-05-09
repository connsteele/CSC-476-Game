#version 330 core 

in vec3 fragNor;
in vec3 WPos;


uniform vec3 eye;
uniform vec3 lightSource;
uniform float hit;

//material uniforms
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

out vec4 color;

vec3 camera = vec3(0.0, 0.75, 9.0); //the position of the surveillance camera

void main()
{
	vec3 lightVec = lightSource - WPos;
	lightVec = normalize(lightVec);

	//calc h
	vec3 V = normalize(eye - WPos); //use eye or camera
	vec3 h = normalize(V + lightVec );
   
   
	vec3 ka = MatAmb;
	vec3 kd = MatDif * clamp(dot(fragNor, lightVec), 0, 1); 
	vec3 ks = MatSpec * pow(dot(h,fragNor),shine); 
   
	color = vec4(clamp(ka + kd + ks, 0, 1), 1.0);
}
