#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform float t;
uniform float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
//set different value to render different FBO effects
uniform int mode;

void main () {
	//blur
	if (mode > 0) {
		vec3 yellow = vec3(0.9, 0.2, 0.0);
		vec3 blue = vec3(0, 0, 0.5);
		vec3 texColor = texture( texBuf, texCoord).rgb;
		//float dist = distance(gl_FragCoord, vec2(320, 240));
		//texColor.xyz += (1.0-dist/400.0)*orange;
		//TODO modify to show this is a 2D image
		if (gl_FragCoord.x > 680)
			//texColor -= blue;
			color = vec4(texColor, 1.0);
		vec4 tempColor = vec4(texture( texBuf, texCoord ).rgb, 1);
		color = tempColor*weight[0];

		 for (int i=1; i <5; i ++) {
			   color += vec4(texture( texBuf, texCoord + vec2(offset[i], 0.0)/512.0 ).rgb, 1)*weight[i];
			   color += vec4(texture( texBuf, texCoord - vec2(offset[i], 0.0)/512.0 ).rgb, 1)*weight[i];
		 }

		 if (tempColor.r > tempColor.b && tempColor.g > tempColor.b) {
			color.xyz += 0.01*yellow;
		 }
	}

	//default
	else {
		vec3 texColor = texture( texBuf, texCoord).rgb;
		color.rgb = texColor.rgb;
    }
}
