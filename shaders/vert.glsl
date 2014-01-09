#version 110 

attribute  vec2 position;	//It's own position offset.

attribute float size;		//Size of an object. Since we draw from -1 to 1, div. by 2.
attribute float Angle;		//It's orientation
attribute  vec2 offset;		//It's world coordinate offset.
uniform float zoom;			//Zzooom

uniform mat4 projection;

varying vec2 texcoord;

void main()
{	

	float cosa = cos(Angle)*size*.5*zoom;
	float sina = sin(Angle)*size*.5*zoom;

	//gl_TexCoord[0] = gl_MultiTexCoord0;

	mat4 RotationMatrix = mat4( cosa, 	-sina, 	0.0, 	0.0,
					      		sina,  	cosa, 	0.0, 	0.0,
			    	      		0.0,	0.0,	1.0, 	0.0,
				          		0.0,	0.0, 	0.0, 	1.0);



	
	vec4 rotated;
	vec4 position4;
	position4 =  vec4(position.x,position.y,1,1);

	//vec4 add = vec4(0.1,0.1,0,0);

	texcoord = vec2(position.x,position.y) * vec2(0.5) + vec2(0.5);
	


	rotated =  (position4*RotationMatrix);
				          
	//vec4 totalOffset = vec4(offset.x*zoom, offset.y*zoom, 0.0, 0.0);
	vec4 totalOffset = vec4(offset.x*zoom, offset.y*zoom, 0.0, 0.0);
	
    gl_Position = rotated + totalOffset;
    
    
}