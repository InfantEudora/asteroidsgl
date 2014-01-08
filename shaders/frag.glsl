#version 110


uniform vec4 color;
//out vec4 outputColor;
//uniform sampler2D colorMap;

uniform sampler2D textures[2];
varying vec2 texcoord;

void main()
{   
	//gl_FragColor = texture2D( colorMap, gl_TexCoord[0].st);
   //gl_FragColor  = color;//vec4(1,1,1,1);

   gl_FragColor = texture2D(textures[0], texcoord);
}
