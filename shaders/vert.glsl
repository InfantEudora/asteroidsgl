#version 120 

attribute vec4 position;	//It's own position offset.

uniform float size;			//Size of object.
uniform float Angle;		//It's orientation
uniform vec2 offset;		//It's world coordinate offset.
uniform float zoom;			//Zzooom

varying vec2 texcoord;

void main()
{	

	float cosa = cos(Angle)*size*zoom;
	float sina = sin(Angle)*size*zoom;

	gl_TexCoord[0] = gl_MultiTexCoord0;

	mat4 RotationMatrix = mat4( cosa, 	-sina, 	0.0, 	0.0,
					      		sina,  	cosa, 	0.0, 	0.0,
			    	      		0.0,	0.0,	1.0, 	0.0,
				          		0.0,	0.0, 	0.0, 	1.0);
	
	vec4 position2;
	//vec4 add = vec4(0.1,0.1,0,0);

	texcoord = vec2(position.x,position.y) * vec2(0.5) + vec2(0.5);
	
	position2 =  (position*RotationMatrix);
				          
	vec4 totalOffset = vec4(offset.x*zoom, offset.y*zoom, 0.0, 0.0);
	
    gl_Position = position2 + totalOffset;
    
    
}