#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
//set different value to render different FBO effects
uniform int mode;

void main () {
	//blur
	if (mode > 0) {
		vec3 texColor = texture( texBuf, texCoord).rgb;
		if (gl_FragCoord.x > 680)
			color = vec4(texColor, 1.0);

		vec4 tempColor = vec4(texture( texBuf, texCoord ).rgb, 1);
		color = tempColor*weight[0];

		 for (int i=1; i < 5; i ++) {
			   color += vec4(texture( texBuf, texCoord + vec2(offset[i], 0.0)/1024.0 ).rgb, 1)*weight[i];
			   color += vec4(texture( texBuf, texCoord - vec2(offset[i], 0.0)/1024.0 ).rgb, 1)*weight[i];
		 }
	}

	//default
	else {
		vec3 texColor = texture( texBuf, texCoord).rgb;
		color.rgb = texColor.rgb;
    }
}
