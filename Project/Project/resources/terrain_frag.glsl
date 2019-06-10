#version 330 core 

// These come from the vertex shader
in vec2 vertex_tex;
in vec3 vertex_nor;
in vec3 vertex_pos;

// Uniforms
uniform vec3 eye;
uniform vec3 lightSource;
uniform sampler2D tex;

out vec4 color;

void main()
{

	vec4 newcolor = texture(tex, vertex_tex);
	// Get rid of the alphas to make it transparent
//	if (newcolor.a < 0.1)
//	{
//		discard;
//	}

	color = newcolor;
	//diffuse lighting
    vec3 n = normalize(vertex_nor);
    vec3 ld = normalize(lightSource - vertex_pos);
    float diffuse = dot(n, ld);
    diffuse = clamp(diffuse, 1, 1.5);
    color *= diffuse;

    // specular lighting
    vec3 cd = normalize(eye - vertex_pos);
    vec3 h = normalize(cd + ld);
    float specular = dot(h, n);
    specular = clamp(specular, 0, 0.5);
    specular = pow(specular, 20);
    color += specular;
    // color.a = 1;
	// Get rid of alpha coloring to make it transparent
	if (color.a < 0.1)
	{
		discard;
	}
   

}
