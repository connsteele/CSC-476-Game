#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;
uniform samplerCube skyBox;
uniform sampler2D normalTex;

uniform vec3 eye;
uniform vec3 lightSource;

//texure uniforms
uniform sampler2D tex;

//material uniforms
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

uniform float hit;

out vec4 Outcolor;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {

    float bias = 0.001;
	int plusmin = 1;
	float divisor = ((plusmin* 2)+1) * ((plusmin* 2)+1);
	float scaleIndex =  8192;
	float sum = 0;
	float xComp;
	float yComp;

	//1: shift the coordinates from -1, 1 to 0 ,1
    vec3 shifted = (LSfPos.xyz + vec3(1)) * 0.5;

	for(int i = -plusmin; i <= plusmin; i++) {
		for(int j = -plusmin; j <= plusmin; j++) {
			xComp = i/scaleIndex;
			yComp = j/scaleIndex;
			//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy
			float Ldepth = texture(shadowDepth, shifted.xy + vec2(xComp, yComp)).r;
			//3: compare to the current depth (.z) of the projected depth
			if (Ldepth < shifted.z - bias)
			    sum += 1.0;
			//4: return 1 if the point is shadowed
		}
	}

	return sum / divisor;
}

void main() {

	float Shade;
	float amb = 0.6;
	vec4 MatColor;
	vec4 AmbColor;


	/*//from terrain
	vec4 newcolor = texture(tex, in_struct.vtexcoord);
	color = newcolor;

	//diffuse lighting
    vec3 n = normalize(in_struct.fragnor);
    vec3 ld = normalize(lightsource - in_struct.fpos);
    float diffuse = dot(n, ld);
    diffuse = clamp(diffuse, 1, 1.5);
	//keep lighting components sep from color
    //color *= diffuse;

    // specular lighting
    vec3 cd = normalize(eye - in_struct.fpos);
    vec3 h = normalize(cd + ld);
    float specular = dot(h, n);
    specular = clamp(specular, 0, 0.5);
    specular = pow(specular, 20);

	//keep lighting components sep from color
    //color += specular;
    //color.a = 1;

	*/

	//from "simple_frag"
	vec3 lightvec = lightSource;
	lightvec = normalize(lightvec);

	//calc h
	vec3 v = normalize(eye - in_struct.fPos); //use eye or camera
	vec3 vsky = normalize(in_struct.fPos - eye); // For use in environment mapping
	vec3 h = normalize(v + lightvec );

	vec3 r = reflect(vsky, normalize(in_struct.fragNor)); // Calculate reflectance use in environment mapping
	vec4 envirmap = vec4(texture(skyBox, r).rgb, 1.0); // Environment map, used in calculation of out color below
   
   
	vec3 ka = MatAmb;
	vec3 kd = MatDif * clamp(dot(in_struct.fragNor, lightvec), 0, 1); 
	vec3 ks = MatSpec * pow(dot(h,in_struct.fragNor),shine); 
   
	MatColor = vec4(clamp(ka + kd + ks, 0, 1), 1.0);
	AmbColor = vec4(MatAmb, 1.0);



	vec4 BaseColor = vec4(in_struct.vColor, 1);
	vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);

	// Discard alphas (dont do this in this shader ill mess up shadows
//	if (texColor0.a < 0.1)
//	{
//		discard;
//	}

	Shade = TestShadow(in_struct.fPosLS);

	 vec3 normal = texture(normalTex, in_struct.vTexCoord).xyz;
	 vec3 shiftedNormal = 2.0 * normal - vec3(1.0);
	 float diffCoef = max(0, dot(normal, lightvec));

	// Clamp our output color to between 0.0 and 1.0
	Outcolor = clamp(amb*(texColor0*AmbColor) + (1.0-Shade) * texColor0 * MatColor * BaseColor * diffCoef * envirmap, 0.0, 1.0);
	//Outcolor = vec4(texture(skyBox, r).rgb, 1.0); // For use in environment mapping
}

