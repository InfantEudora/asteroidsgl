#include "Player.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "asteroids_common.h"

#if COMPILE_SOUND
extern void sound_play(int,int);
#endif

/* 
	Simple: Loads a bitmap and ignores any header information.
*/
int LoadFileBitmap(char* path, uint8_t* target, uint32_t size){
	uint8_t header[54];

	FILE* file = fopen(path,"rb");
	if (!file){
		printf("ship.bmp could not be opened.\n");
		return -1;
	}
	if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return -1;
	}
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return -1;
	}

	// Read the actual data from the file into the buffer
	fread(target,1,size,file);

	//Everything is in memory now, the file can be closed
	fclose(file);

}

/* 
	Simple: Load a TGA file and ignores the header.
*/
int LoadFileTGA(char* path, uint8_t* target, uint32_t size){
	uint8_t header[18];

	FILE* file = fopen(path,"rb");
	if (!file){
		printf("ship.bmp could not be opened.\n");
		return -1;
	}
	if ( fread(header, 1, 18, file)!=18 ){ // If not 54 bytes read : problem
		printf("Not a correct TGA file\n");
		return -1;
	}

	// Read the actual data from the file into the buffer
	fread(target,1,size,file);

	//Everything is in memory now, the file can be closed
	fclose(file);

}

//Returns length of a vector.
float vect_abs(vect2* a){	
	return sqrtf((a->x*a->x)+(a->y*a->y));	
}

//Returns the square of the length of a vector.
float vect_abs_sqare(vect2* a){	
	return (a->x*a->x)+(a->y*a->y);	
}

//Scales a vector.
vect2 vect_scale(vect2* a,float scale){
	vect2 t = *a;
	t.x *= scale;
	t.y *= scale;
	return t;
}

//Returns the unit vector of a.
vect2 vect_unit(vect2* a){
	vect2 t = *a;
	float size = vect_abs(a);
	if (size == 0){
		return t;
	}
	t.x /= size;
	t.y /= size;
	return t;
}

//Returns the orientation of a vector in degrees
float vect_ordeg(vect2* a){	
	float t = atan2(a->y,a->x) + PI/2;
	t = (t* 360) / (2*3.1415926535 );
	//printf("angle: % 6.2f\n",t);
	return t;
}

//Returns the difference between two vectors as a vector.
vect2 vect_diff(vect2* a, vect2* b){
	vect2 t;
	t.x = a->x - b->x;
	t.y = a->y - b->y;
	return t;
}

//Returns the sum of 2 vectors as a vector.
vect2 vect_sum(vect2* a, vect2* b){
	vect2 t;
	t.x = a->x + b->x;
	t.y = a->y + b->y;
	return t;
}

//Reads a file and returns a pointer to newly allocated memory.
void *file_contents(const char *filename, GLint *length){
    FILE *f = fopen(filename, "r");
    void *buffer;

    if (!f) {
        printf("ERREUR!: Unable to open %s for reading\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return buffer;
}

//Checks if two cubes overlap.
//Vectors are the center, size is the size of a side.
bool inrange_cube(vect2* a, vect2* b, float* size_a, float* size_b){
	float sza = *size_a/2;
	float szb = *size_b/2;
	if (a->x-sza < b->x+szb){
		if (a->x+sza > b->x - szb){
			if (a->y-sza < b->y+szb){
				if (a->y+sza > b->y - szb){					
					return true;
				}
			}
		}
	}
	return false;
}

 Obj::Obj(){

 }

 Obj::~Obj(){

 }

void Obj::init(vect2* pos, float* rot, float* sz){
	position = pos;
	rotation = rot;
	size = sz;

	velocity.x = 0;
	velocity.y = 0;

	momentum.x = ((rand()%100)-50)/10.0f;
	momentum.y = ((rand()%100)-50)/10.0f;

	//momentum.x =1;
	//momentum.y =1;

	*size = ((rand()%500)+500)/1000.0f;
	mass = (*size) * 15;

	velocity.x = momentum.x / mass;
	velocity.y = momentum.y / mass;

	//position->x = 0;
	//position->y = 0;

	*rotation = 1;
	drot =  ((rand()%500)+500)/10000.0f;

	

	//*size = 1;
}

void Obj::updateMomentum(void){
	momentum.x = velocity.x * mass;
	momentum.y = velocity.y * mass;
}


void set_attraction_force(Obj *a, Obj* b){
	float G = .7;	


	vect2 diff = vect_diff(a->position,b->position);

	//Force between a and b.
	float force = G*(a->mass*b->mass)/vect_abs_sqare(&diff);

	//Make a vector
	vect2 vec_f;
	vec_f.x = 1;
	vec_f.y = 0;

	//Rotate
	float ang = atan2(diff.y,diff.x);

	a->force.x += -cosf(ang)*force;
	a->force.y += -sinf(ang)*force;


	b->force.x += cosf(ang)*force;
	b->force.y += sinf(ang)*force;
}



 /*
	Does physics that are appicable to any object.
	An inherited class must call this funtion at the end of it's own doPhysics.
 */
 void Obj::doObjPhysics(float ticks){
	float dt = ticks/20000;
	float constant = 0.04f;
	float dvx,dvy,dx,dy,dfx,dfy;
	
	dfx = force.x * dt;
	dfy = force.y * dt;

	//Change in acceleration
	acceleration.x = force.x / mass;
	acceleration.y = force.y / mass;

	//Change in velocity:
	dvx = acceleration.x * dt;
	dvy = acceleration.y * dt;

	//Change in position
	dx = velocity.x * dt;
	dy = velocity.y * dt;

	//Update position with old velocity.
	(*position).x += dx*constant;
	(*position).y += dy*constant;

	//Update velocity with old acceleration.
	velocity.x += dvx*constant;
	velocity.y += dvy*constant;

	//Update acceleration with old force.
	//acceleration.x += dfx*constant;
	//acceleration.y += dfy*constant;
	
	//Update momentum
	updateMomentum();

	//Apply some speed limits.
	if (vect_abs_sqare(&velocity) > 100){
		momentum.x /= 2;
		momentum.y /= 2;
		velocity.x /= 2;
		velocity.y /= 2;
	}

	//The rest.
	*rotation +=drot;
 }

 /*
	Check if you have collided with another object.
 */
bool Obj::checkCollision(Obj* obj){
	return inrange_cube(this->position,obj->position,this->size,obj->size);
}
  
void ObjManager::doPhysics(float ticks){
	//Rotate mah balllz.
	/*
	for(int i =0;i<num_balls;i++){
		for(int r =0;r<8;r++){
			g_vertex_rotation[(i*8)+r] += 0.1f;
		}
	}*/
	//Let there be gravity, reset all force:
	for (int i=0;i<num_balls;i++){
		object[i].force.x= 0;
		object[i].force.y= 0;
	}
	//Check collision.
	for (int i=0;i<num_balls;i++){
		object[i].doObjPhysics(ticks);
		for (int r=i+1;r<num_balls;r++){
			//Set attraction force.
			//set_attraction_force(&object[i],&object[r]);
			if (object[i].checkCollision(&object[r])){
				
				//The difference in position.
				vect2 dpos = vect_diff(object[i].position,object[r].position);
								
				//If the radiusses overlap:				
				float dposabs = vect_abs(&dpos);				

				float overlap = dposabs;
				overlap -= (*(object[r].size)/2); //This would be the radius.
				overlap -= (*(object[i].size)/2);
				
				if (overlap< 0){
					if ((i==1)|| (r==1)){
						sound_play(SND_BOUNCE,50);
					}

					//Their radiusses overlap. dposabs is the length of overlap.
					vect2 unit = vect_unit(&dpos);

					//Get a unit vector of the difference.
					vect2 displace = vect_scale(&unit,abs(overlap/2));
							
					//Apply a 50/50 displacement to both objects.					
					(*object[r].position).x -= displace.x;
					(*object[r].position).y -= displace.y;
					(*object[i].position).x += displace.x;
					(*object[i].position).y += displace.y;
					
					/*	
					 In a perfectly elastic collision, kinetic energy is conserved.
					 In a perfectly inelastic collision, the momentum is conseverd.
					 
					 We must calculate each balls velocity by rotation their vectors to
					 a collision frame of reference. 
					 We can then find their 1D velocity change along this reference, and 
					 rotate back. 

					 obj[i] = a, obj[r] = b;
					*/
					//break;
					float m1 = object[i].mass;
					float m2 = object[r].mass;

					//Angle by which to rotate so we get an x-aligned reference frame:
					float dir_col = atan2(dpos.y,dpos.x)-(PI/2);

					//Get values and direction:					
					vect2 vel_ia = object[i].velocity;
					float dir_ia = atan2(vel_ia.y,vel_ia.x);
					float mag_ia = vect_abs(&vel_ia);
					
					vect2 vel_ib = object[r].velocity;
					float dir_ib = atan2(vel_ib.y,vel_ib.x);
					float mag_ib = vect_abs(&vel_ib);
					
					//Transpose
					vect2 vel_ta;
					vect2 vel_tb;

					if (dir_col < (0)){
						dir_col = (-PI/2) - dir_col;
					}else{
						dir_col  = (PI/2) - dir_col;
					}

					float rot_backa = dir_col;
					float rot_backb = dir_col;

					//vel_ta.x = mag_ia * cos(rot_backa);
					//vel_ta.y = mag_ia * sin(rot_backa);

					//vel_tb.x = mag_ib * cos(rot_backb);
					//vel_tb.y = mag_ib * sin(rot_backb);

					vel_ta.x =  (object[i].velocity.x * cosf(rot_backa)) - (object[i].velocity.y * sinf(rot_backa));
					vel_ta.y =  (object[i].velocity.x * sinf(rot_backa)) + (object[i].velocity.y * cosf(rot_backa));

					vel_tb.x =  (object[r].velocity.x * cosf(rot_backb)) - (object[r].velocity.y * sinf(rot_backb));
					vel_tb.y =  (object[r].velocity.x * sinf(rot_backb)) + (object[r].velocity.y * cosf(rot_backb));


					//Apply 1D collision:
					float vel_tfax;
					float vel_tfbx;
					float vel_cm;

					vel_cm = ((m1*vel_ta.x) + (m2*vel_tb.x))/(m1+m2);

					vel_tfax = vel_ta.x - vel_cm;
					vel_tfbx = vel_tb.x - vel_cm;

					vel_tfax = -vel_tfax;
					vel_tfbx = -vel_tfbx;

					vel_tfax += vel_cm;
					vel_tfbx += vel_cm;


					//vel_tfax = ((vel_ta.x*(m1-m2))+ (2* m2 *vel_tb.x))/(m1+m2);
					//vel_tfbx = ((vel_tb.x*(m1-m2))+ (2* m2 *vel_ta.x))/(m1+m2);

					//Now rotate everything back:
					vect2 vel_fa;
					vect2 vel_fb;

					vel_fa.x = vel_tfax;
					vel_fa.y = vel_ta.y;

					vel_fb.x = vel_tfbx;
					vel_fb.y = vel_tb.y;

					rot_backa = -rot_backa;
					rot_backb = -rot_backb;
					
					object[i].velocity.x = (vel_fa.x * cosf(rot_backa)) - (vel_fa.y * sinf(rot_backa));
					object[i].velocity.y = (vel_fa.x * sinf(rot_backa)) + (vel_fa.y * cosf(rot_backa));

					object[r].velocity.x = (vel_fb.x * cosf(rot_backb)) - (vel_fb.y * sinf(rot_backb));
					object[r].velocity.y = (vel_fb.x * sinf(rot_backb)) + (vel_fb.y * cosf(rot_backb));
					
					object[i].updateMomentum();
					object[r].updateMomentum();
					/*
										
					//Set their velocity based on angle of collision:
					//i:
					float ang_impi = atan2(object[i].velocity.y,object[i].velocity.x);
					float ang_norm = atan2(dpos.y,dpos.x);
					
					float ang_dif = -2*(ang_impi - ang_norm);

					vect2 vel_ai = object[i].velocity;
					vect2 vel_afu; //A's final momentum unit vector (direction).

					object[i].velocity.x = (vel_ai.x * cosf(ang_dif)) - (vel_ai.y * sinf(ang_dif));
					object[i].velocity.y = (vel_ai.x * sinf(ang_dif)) + (vel_ai.y * cosf(ang_dif));
					//Create the unit vector.
					vel_afu = vect_unit(&object[i].velocity);
					//r:
					float ang_impr = atan2(object[r].velocity.y,object[r].velocity.x);
					ang_norm = atan2(dpos.y,dpos.x);
				
					ang_dif =-2*(ang_impr - ang_norm);
				
					vect2 vel_bi = object[r].velocity;
					vect2 vel_bfu; //B's final momentum unit vector (direction).

					object[r].velocity.x = (vel_bi.x * cosf(ang_dif)) - (vel_bi.y * sinf(ang_dif));
					object[r].velocity.y = (vel_bi.x * sinf(ang_dif)) + (vel_bi.y * cosf(ang_dif));
					//Create the unit vector.
					vel_bfu = vect_unit(&object[r].velocity);

					/*
					 These where actually just for getting their directions.
					 Now we need to conserve the momentum: Pai + Pbi = Paf + Pbf
					*/
					//vect2 vel_sumi = vect_sum(&vel_ai,&vel_bi);
					//float vel_sumi_abs = vect_abs(&vel_sumi);
					//float mass_tot = object[i].mass + object[r].mass;

					/*
					 Both objects get a fraction of the mommentum depending on their mass relation:
					 Eg. if the mass is the same, both end up with the same speed.
					 If a small moving object hits a big stationary one, the speed of the small object decreases significantly,
					 while the big objects speed increases only slightly.
					*/

					//object[i].velocity = vect_scale(&vel_afu,(object[i].mass/mass_tot)*vel_sumi_abs);
					//object[r].velocity = vect_scale(&vel_bfu,(object[r].mass/mass_tot)*vel_sumi_abs);

					//object[i].updateMomentum();
					//object[r].updateMomentum();

				}
			}
		}

		//Check bounds
		int bound = 20.0f;
		int bound_hyst = 0;

		/*
		if ((*object[i].position).x > (bound)){
			(*object[i].position).x = -(bound);
		}
		if ((*object[i].position).x < -(bound+bound_hyst)){
			(*object[i].position).x = +(bound-bound_hyst);	
		}
		if ((*object[i].position).y > (bound+bound_hyst)){
			(*object[i].position).y = -(bound-bound_hyst);	
		}
		if ((*object[i].position).y < -(bound)){
			(*object[i].position).y = (bound);	
		}*/
		
		if ((*object[i].position).x > (bound)){
			object[i].velocity.x = -1 * abs(object[i].velocity.x);
		}
		if ((*object[i].position).x < -(bound)){
			object[i].velocity.x = 1 * abs(object[i].velocity.x);
		}
		if ((*object[i].position).y > (bound)){
			object[i].velocity.y = -1 * abs(object[i].velocity.y);	
		}
		if ((*object[i].position).y < -(bound)){
			object[i].velocity.y = 1 * abs(object[i].velocity.y);
		}
		object[i].updateMomentum();
	}

	//Do obj physics, and update vertext data for rendering.
	for (int i=0;i<num_balls;i++){
		

		//Copy vertex data.		
		g_vertex_location[(8*i)+2] = g_vertex_location[(8*i)+0];
		g_vertex_location[(8*i)+3] = g_vertex_location[(8*i)+1];
		g_vertex_location[(8*i)+4] = g_vertex_location[(8*i)+0];
		g_vertex_location[(8*i)+5] = g_vertex_location[(8*i)+1];
		g_vertex_location[(8*i)+6] = g_vertex_location[(8*i)+0];
		g_vertex_location[(8*i)+7] = g_vertex_location[(8*i)+1];

		//Rotation
		g_vertex_rotation[(4*i)+1] = g_vertex_rotation[(4*i)+0];
		g_vertex_rotation[(4*i)+2] = g_vertex_rotation[(4*i)+0];
		g_vertex_rotation[(4*i)+3] = g_vertex_rotation[(4*i)+0];

		//Rotation
		g_vertex_size[(4*i)+1] = g_vertex_size[(4*i)+0];
		g_vertex_size[(4*i)+2] = g_vertex_size[(4*i)+0];
		g_vertex_size[(4*i)+3] = g_vertex_size[(4*i)+0];
	}
 }

ObjManager::ObjManager(){
	//Coordinates for balls.
	num_balls = MAX_NUM_OBJ;
	for(int i =0;i<num_balls*2;i++){
		b_position[i] = (rand()%10000-5000)/500.0f;
	}

	


	//Orientation for the balls.
	/*
	for(int i =0;i<num_balls;i++){
		b_rotation[i] = rand()%360;
		b_size[i] = (rand()%100+20)/800.0f;
	}*/

	//Just a square.
	//GLfloat g_vertex_data[8] = {-1,-1,-1,1,1,1,1,-1};


	color4[0] = 1;
	color4[1] = 1;
	color4[2] = 1;
	color4[3] = 1;

	orientation = 0;
	zoom = .2;

	b_position[0] =.1;
	b_position[1] =.1;
	b_position[2] =.2;
	b_position[3] =.2;

	for (int i=0;i<num_balls;i++){
		g_vertex_data[(8*i)+0] = -1;
		g_vertex_data[(8*i)+1] = -1;
		g_vertex_data[(8*i)+2] = -1;
		g_vertex_data[(8*i)+3] = 1;
		g_vertex_data[(8*i)+4] = 1;
		g_vertex_data[(8*i)+5] = 1;
		g_vertex_data[(8*i)+6] = 1;
		g_vertex_data[(8*i)+7] = -1;

		
	
		g_vertex_location[(8*i)+0] = b_position[(2*i)];
		g_vertex_location[(8*i)+1] = b_position[(2*i)+1];
		g_vertex_location[(8*i)+2] = b_position[(2*i)];
		g_vertex_location[(8*i)+3] = b_position[(2*i)+1];
		g_vertex_location[(8*i)+4] = b_position[(2*i)];
		g_vertex_location[(8*i)+5] = b_position[(2*i)+1];
		g_vertex_location[(8*i)+6] = b_position[(2*i)];
		g_vertex_location[(8*i)+7] = b_position[(2*i)+1];

		g_vertex_rotation[(4*i)+0] = 0;
		g_vertex_rotation[(4*i)+1] = 0;
		g_vertex_rotation[(4*i)+2] = 0;
		g_vertex_rotation[(4*i)+3] = 0;


		g_vertex_size[(4*i)+0] = 1;
		g_vertex_size[(4*i)+1] = 1;
		g_vertex_size[(4*i)+2] = 1;
		g_vertex_size[(4*i)+3] = 1;

	}

	//Bind objects.
	vect2* ptr_locmem = (vect2*)&g_vertex_location[0];
	//ptr_locmem = (vect2*)&g_vertex_location[8];

	float* ptr_rotmem =  (float*)&g_vertex_rotation[0];
	float* ptr_sizemem = (float*)&g_vertex_size[0];
	for (int i=0;i<num_balls;i++){
		object[i].init(ptr_locmem,ptr_rotmem,ptr_sizemem);
 		int s = 4*sizeof(vect2);
		ptr_locmem+= 4;
		ptr_rotmem += 4;
		ptr_sizemem += 4;
	}

	//Test 5 balls.
	(*object[0].size) = 2;
	object[0].mass = 50;
	(*object[0].position).x = -0;
	(*object[0].position).y = -0;
	object[0].velocity.x = 0;
	object[0].velocity.y = 0;
	object[0].updateMomentum();  

	(*object[1].size) = .5;
	object[1].mass = 1.5;
	(*object[1].position).x = -3;
	(*object[1].position).y = .22;
	object[1].velocity.x = 1/2.00;
	object[1].velocity.y = -.1;
	object[1].updateMomentum();
	/*
	(*object[2].size) = 1;
	object[2].mass = 1;
	(*object[2].position).x = -2;
	(*object[2].position).y = 5;
	object[2].velocity.x = 1/10.0;
	object[2].velocity.y = -5/10.0;
	object[2].updateMomentum();

	(*object[3].size) = 1;
	object[3].mass = 1;
	(*object[3].position).x = -2;
	(*object[3].position).y = -2;
	object[3].velocity.x = 5/10.0;
	object[3].velocity.y = 5/10.0;
	object[3].updateMomentum();

	(*object[4].size) = 1;
	object[4].mass = 1;
	(*object[4].position).x = 2;
	(*object[4].position).y = -2;
	object[4].velocity.x = -8/10.0;
	object[4].velocity.y = 0/10.0;
	object[4].updateMomentum();
	*/
	
};
ObjManager::~ObjManager(){};


void ObjManager::init(){
	init_shaders();
	init_textures();
}

void ObjManager::init_textures(){
	//Generate texture
	LoadFileTGA("bitmaps/rock2.tga",texture_data_rock, TGA_SIZE_256);
	LoadFileTGA("bitmaps/ball.tga",texture_data_ball, TGA_SIZE_256);
	LoadFileTGA("bitmaps/nebula2.bmp",texture_data_back_1, BMP_SIZE_1024);

	glGenTextures(3, &textures[0]);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_data_rock);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_data_ball);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_data_back_1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
}

void ObjManager::init_shaders(){
	//Init glew, and ignore what it returns.
	glewInit();
	
	vertex_shader = make_shader(GL_VERTEX_SHADER,"shaders/vert.glsl");
	fragment_shader = make_shader(GL_FRAGMENT_SHADER,"shaders/frag.glsl");
	shader_program = make_program(vertex_shader,fragment_shader);

	if (shader_program == 0) {
		printf("Failed to create shader_program.\n");		
	}

	//ball.init_resources(shader_program);

	



	////////////////From here
	

	 // vertex buffer
	glGenBuffersARB(1, &vertex_buffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,vertex_buffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(g_vertex_data),g_vertex_data,GL_STATIC_DRAW);

	//location buffer
	glGenBuffersARB(1, &location_buffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,location_buffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(g_vertex_location),g_vertex_location,GL_STATIC_DRAW);

	// element buffer
	glGenBuffersARB(1, &rotation_buffer);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,rotation_buffer);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,sizeof(g_vertex_rotation),g_vertex_rotation,GL_STATIC_DRAW);

	// size buffer
	glGenBuffersARB(1, &size_buffer);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,size_buffer);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,sizeof(g_vertex_size),g_vertex_size,GL_STATIC_DRAW);


	// vertex shader arguments
	va_scale = glGetAttribLocation (shader_program, "size");
	va_position = glGetAttribLocation(shader_program, "position");	
	va_rotation = glGetAttribLocation(shader_program, "Angle");
	va_location = glGetAttribLocation(shader_program, "offset");

	//va_textures[0]= glGetUniformLocation(shader_program, "textures[0]");
	//va_textures[1]= glGetUniformLocation(shader_program, "textures[1]");

	va_zoom = glGetUniformLocation(shader_program, "zoom");
	va_relative = glGetUniformLocation(shader_program, "relative");

	//Fragent shader.
	va_color = glGetUniformLocation(shader_program, "color");	
} 

GLuint ObjManager::make_shader(GLenum type, char *filename) {
	GLint length;
	GLchar *source = (GLchar*) file_contents(filename, &length);
	GLuint shader;
	GLint shader_ok;

	if (!source){
		printf("Could not find file\n");		
	}

	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**) &source, &length);
	free(source);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
	if (!shader_ok) {
		fprintf(stderr, "Failed to compile %s:\n", filename);
        //show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
	}
	return shader;
}

GLuint ObjManager::make_program(GLuint vertex_shader, GLuint fragment_shader) {
	GLint program_ok;

	GLint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
	if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        //show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
	}	

	return program;
}

void ObjManager::Rotate(float by, float t){
	by /= 4.0f;
	//orientation += by;
	//	this->ball.orientation += by;
	float a = vect_abs(&object[1].velocity);
	float ang = atan2(object[1].velocity.y,object[1].velocity.x);
	object[1].velocity.x = cosf(ang+by)* a;
	object[1].velocity.y = sinf(ang+by)* a;
	object[1].updateMomentum();

}

void ObjManager::Move(float by, float t){
	by /= 40.0f;
	//orientation += by;
	//	this->ball.orientation += by;
	float a = vect_abs(&object[1].velocity);
	float ang = atan2(object[1].velocity.y,object[1].velocity.x);
	a+= by;

	object[1].velocity.x = cosf(ang)* a;
	object[1].velocity.y = sinf(ang)* a;
	object[1].updateMomentum();

}

void ObjManager::render(void){
    //glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_BLEND);

	//Backdrop?
	//glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0,0,-1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glBegin(GL_QUADS);		
		glColor4f(1,1,1,0.1);
		glTexCoord2f(0,0);		glVertex3f(-1, -1, .0f);
		glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
		glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
		glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);		
	glEnd();
	glPopMatrix();

	
	
	//Normal-ish.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(shader_program);

	glEnableVertexAttribArray(va_location);	
	glEnableVertexAttribArray(va_position);	
	glEnableVertexAttribArray(va_rotation);	
	glEnableVertexAttribArray(va_scale);	
	

	
	//2-Triangle coordinates.
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vertex_buffer);	
	glVertexAttribPointer( va_position,
				2,                             
				GL_FLOAT,                         
				GL_FALSE,                        
				0,//sizeof(GLfloat)*2,               
				(void*)0);


	//2-Triangle coordinates.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, location_buffer);	
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(g_vertex_location),g_vertex_location,GL_DYNAMIC_DRAW_ARB);
	
	
	glVertexAttribPointer( va_location,
				2,                             
				GL_FLOAT,                         
				GL_FALSE,                        
				0,//sizeof(GLfloat)*2,               
				(void*)0);

	//Rotation.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, rotation_buffer);	
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(g_vertex_rotation),g_vertex_rotation,GL_DYNAMIC_DRAW_ARB);	
	glVertexAttribPointer( va_rotation,
				1,                             
				GL_FLOAT,                         
				GL_FALSE,                        
				0,//sizeof(GLfloat),
				(void*)0);

	//Size.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, size_buffer);	
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(g_vertex_size),g_vertex_size,GL_DYNAMIC_DRAW_ARB);	
	glVertexAttribPointer( va_scale,
				1,                             
				GL_FLOAT,                         
				GL_FALSE,                        
				0,//sizeof(GLfloat),
				(void*)0);


	//Textures
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    //glUniform1i(textures[0], 0);


	//Elements
	/*
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, element_buffer);		
	for(int i =0;i<num_balls;i++){
		
		glUniform2f(va_location, b_position[0+i],b_position[1+i]);
		glUniform1f(va_rotation, b_rotation[i]);
		glUniform1f(va_scale, b_size[i]);
		glDrawElements(
			GL_TRIANGLE_STRIP,
			4,
			GL_UNSIGNED_SHORT,
			(void*)0          
		);
	}*/
	
	//glUniform2f(va_location, b_position[0],b_position[0]);
	//glUniform1f(va_rotation, b_rotation[0]);
	glUniform1f(va_zoom, zoom);
	glUniform2f(va_relative,-(*object[1].position).x,-(*object[1].position).y);
	//glUniform1f(va_scale, 1);

//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, element_buffer);				
	/*
	glDrawElements(
			GL_TRIANGLES,
			3*2,
			GL_UNSIGNED_SHORT,
			(void*)0          
		);*/
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vertex_buffer);

	//0th ball is green
	glUniform4f(va_color,.5,1,.5,1);		
	glDrawArrays(GL_QUADS,0,4);

	//The rest is normal
	glUniform4f(va_color,1,1,1,1);		
	glDrawArrays(GL_QUADS,4,4*(num_balls-1));

	/*
	 glBindTexture(GL_TEXTURE_2D, textures[0]);
	for(int i =50;i<100;i++){
		glDrawArrays(GL_QUADS,i*100*4,4*100);
	}
	*/
	//glDrawArrays(GL_LINE_STRIP,0,5);


	glDisableVertexAttribArray(va_scale);	
	glDisableVertexAttribArray(va_location);	
	glDisableVertexAttribArray(va_position);	
	glDisableVertexAttribArray(va_rotation);	

	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);	

	glUseProgram(0);

	

}











Ship::Ship(){
	//Amount of ticks a calculation is normally 100/100 = 1;
	ticks_normal = 20000;
	friction_normal = 1;
	
	//Friction slows the inertia.
	friction = 2;
	
	mass = 10;

	//Position in x,y.
	position.x = 0;
	position.y = 0;
	
	//Rotates the orient vector.
	anglerad = 0;
	angledeg = 0;

	//Vertor of inertia.
	inertia.x = 0;
	inertia.y = 0;

	//Unit vector in which move adds.
	orient.x = 1;
	orient.y = 0;

	accel.x = 0;
	accel.y = 0;

	velocity.x = 0;
	velocity.y = 0;

	force.x = 0;
	force.y = 0;

	color.r = 1;
	color.b = 0;
	color.g = 0;

	zoom = 0;

	shield_enabled = true;
	shield_auto_enable = true;

	shield = 200;
	shield_max = 200;	
	shield_regen = 0.012f;	

	health = 100;
	health_max = 100;
	health_regen = 0.0f;

	energy = 50;
	energy_max = 150;
	energy_regen = 0.4f;

	angular_momentum = 0;

	alive = false;

	anim_shield_washit = 0;

	//Orientate
	RotateDeg(0,ticks_normal);

	//Generate texture
	LoadFileTGA("bitmaps/ship_1.tga",bmpdata, TGA_SIZE_256);

	glGenTextures(1, &textureid);
	glBindTexture(GL_TEXTURE_2D, textureid);



	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, bmpdata);

	//Generate texture
	LoadFileTGA("bitmaps/shieldbubble.tga",shield_data[0], TGA_SIZE_256);
	LoadFileTGA("bitmaps/shieldbubble_edge.tga",shield_data[1], TGA_SIZE_256);

	glGenTextures(2, &shieldtext[0]);
	glBindTexture(GL_TEXTURE_2D, shieldtext[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, shield_data);	

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	

	//Engine texture
	LoadFileTGA("bitmaps/flame1.bmp",engine_data, BMP_SIZE_256);

	glGenTextures(1, &enginetext);
	glBindTexture(GL_TEXTURE_2D, enginetext);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_BGR, GL_UNSIGNED_BYTE, engine_data);	

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	

}

void Ship::LoadShip(char* name){
	LoadFileBitmap(name,bmpdata, BMP_SIZE_256);
}

void Ship::DoDamage(float damage){
	float damage_remaining = damage;

	if (shield_enabled){
		if (shield > 0){
			//Play animation.
			anim_shield_washit = 1;
			anim_shield_washit_fact = 1/(damage*20);
			if (anim_shield_washit_fact < 0.001f){
				anim_shield_washit_fact = 0.001f;
			}

			//Play sound
			#if COMPILE_SOUND
			float level = 5*damage;
			if (level > 40){
				level = 40;
			}
			sound_play(SND_SHIELD,level);
			#endif

			shield -= damage_remaining;
			damage_remaining = 0;
			if (shield < 0){
				damage_remaining -= shield;
				shield = 0;
				shield_enabled = false;
			}
		}
	}

	health -= damage_remaining;
	if (health < 0){
		health = 0;
	}		
}


Ship::~Ship(){
	
}

void Ship::Fire(float impulse){
	//Update these, just to be sure.
	weapon.position = position;
	weapon.orient = orient;

	if (energy > impulse){
		energy-= impulse;
		//Fire weapon.
		weapon.Fire(velocity,impulse);
		#if COMPILE_SOUND
		sound_play(SND_WEAPON,50);
		#endif
	}	
}

void Ship::Render(vect2 relative, bool center){
	if (!alive){
		return;
	}


	glPushMatrix();
		
	if (center){
		glTranslatef(-relative.x,-relative.y,zoom);		
	}else{
		glTranslatef(position.x-relative.x,position.y-relative.y,zoom);
	}	
	
		
	glRotatef(angledeg,0.f,0.f,1.0f);
	//glGenTextures(1, &textureid);
	//glBindTexture(GL_TEXTURE_2D, textureid);
	//glTexEnvf(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, bmpdata);

	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);					// Set The Blending Function For Translucency
	
	
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, textureid);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);		glVertex3f(-1, -1, 0.0f);
		glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
		glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
		glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);
	glEnd();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_BGR, GL_UNSIGNED_BYTE, engine_data);
	//glScalef(2,2,1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	
	glRotatef(180,0.f,0.f,1.0f);
	glScalef(1,0.5,1);
	glTranslatef(.37,1.7*1.5,0);
	
	glBegin(GL_QUADS);
		//Engines
		float t = vect_abs(&accel)*7;
		float tr = (rand()%255)/255.0f;
		glColor4f(tr,1,1,t);
		glTexCoord2f(0,0);		glVertex3f(-1, -1, 0.0f);
		glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
		glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
		glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);
		
	glEnd();
	glTranslatef(-1,0,0);
	glBegin(GL_QUADS);
		//Engines
		t = vect_abs(&accel)*7;
		glColor4f(tr,1,1,t);
		glTexCoord2f(0,0);		glVertex3f(-1, -1, 0.0f);
		glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
		glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
		glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);
		
	glEnd();
	//Undo transforms.
	glTranslatef(-.37+1,-1.7*1.5,0);
	glScalef(1,2,1);
	glRotatef(-180,0.f,0.f,1.0f);	
	//Rotate back.
	glRotatef(-angledeg,0.f,0.f,1.0f);


	//Render stats.
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);	
		//Bar base.
		glColor4f(1,1,1,0.2f);
		glVertex3f(-1, 1, -0.0f);	
		glVertex3f(1.0, 1, -0.0f);	
		glVertex3f(1.0, 1.1, -0.0f);	
		glVertex3f(-1, 1.1, -0.0f);
		//Hull value.
		glColor4f(0,1,0,0.5f);
		glVertex3f(-1, 1, -0.0f);	
		glVertex3f(((health*2)/health_max)-1, 1, -0.0f);	
		glVertex3f(((health*2)/health_max)-1, 1.05, -0.0f);	
		glVertex3f(-1, 1.05, -0.0f);
		//Shield value.
		glColor4f(0,0.5,1,0.5f);
		glVertex3f(-1, 1.05, -0.0f);	
		glVertex3f(((shield*2)/shield_max)-1, 1.05, -0.0f);	
		glVertex3f(((shield*2)/shield_max)-1, 1.10, -0.0f);	
		glVertex3f(-1, 1.10, -0.0f);
	glEnd();

	//Render shields.
	if (shield_enabled){

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, &shield_data[1]);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

		glEnable(GL_TEXTURE_2D);
		
		//Translucent blending.
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
		
		glDisable(GL_DEPTH_TEST);

		glBindTexture(GL_TEXTURE_2D, shieldtext[1]);
		
		//Animate.
		float sa = rand()%360;
		glRotatef(sa,0.f,0.f,1.0f);

		glScalef(1.2,1.2,1.0);

		//Draw shields.
		glColor4f(1,1,1,0.6*(shield/shield_max));
		glBegin(GL_QUADS);
			 glTexCoord2f(0,0);
			 glVertex3f(-1, -1, -0.0f);
			 glTexCoord2f(1.0, 0);
			 glVertex3f(1.0, -1, -0.0f);
			 glTexCoord2f(1.0, 1.0);
			 glVertex3f(1.0, 1.0, -0.0f);
			 glTexCoord2f(0, 1.0);
			 glVertex3f(-1, 1.0, -0.0f);
		glEnd();

		//Load bubble
		glBindTexture(GL_TEXTURE_2D, shieldtext[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, shield_data[0]);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		
		//Draw shields.
		if (anim_shield_washit == 0){
			glColor4f(1,1,1,0.05*(shield/shield_max));
		}else{
			glColor4f(1,1,1,0.5*(shield/shield_max)*anim_shield_washit);
		}
		glBegin(GL_QUADS);
			 glTexCoord2f(0,0);
			 glVertex3f(-1, -1, -0.0f);
			 glTexCoord2f(1.0, 0);
			 glVertex3f(1.0, -1, -0.0f);
			 glTexCoord2f(1.0, 1.0);
			 glVertex3f(1.0, 1.0, -0.0f);
			 glTexCoord2f(0, 1.0);
			 glVertex3f(-1, 1.0, -0.0f);
		glEnd();

	}

	
	glEnable(GL_DEPTH_TEST);
	
	glPopMatrix();	
	
	//vect2 wpos;
	//wpos.x = position.x - relative.x;
	//wpos.y = position.y - relative.y;
	weapon.Render(relative);
}


void Ship::EnableShield(bool enable){
	if (enable){
		//Play hit animation.
		anim_shield_washit = 1;
		anim_shield_washit_fact = 0.002f;
	}
	shield_enabled = enable;
	shield_auto_enable = enable;
}


void Ship::RotateDeg(float angle, float ticks){

	float dt = ticks/ticks_normal;

	angledeg += angle*dt;
	anglerad = (angledeg / 360.0f)*2*3.1415926535;
	//
	orient.x = -sinf(anglerad);
	orient.y = cosf(anglerad);
}

//Adds angular momentum
void Ship::RotateImpulse(float impulse, float ticks){
	float dt = ticks/ticks_normal;

	energy -= abs(impulse)*dt*.4f;
	if (energy < 0){
		impulse = impulse+energy;
		energy = 0;
	}

	angular_momentum += impulse*dt;	
}

void Ship::Move(float value, float ticks){	
	float dt = ticks/ticks_normal;
	float ecost = abs(value)*dt;	

	energy -= ecost;
	if (energy < 0){
		float erem = abs(energy)/ecost;
		value = value*erem;
		energy = 0;
	}

	//Add to the ships inertia.	
	accel.x += orient.x*value*dt/5;
	accel.y += orient.y*value*dt/5;

	//printf("Accel.x  : %8.3f\n",accel.x);
	//printf("Accel.y  : %8.3f\n",accel.y);
	//printf("Accel.abs: %8.3f\n",vect_abs(&accel));
}

void Ship::Strafe(float value, float ticks){	
	float dt = ticks/ticks_normal;
	float ecost = abs(value)*dt;	
	energy -= ecost;
	
	if (energy < 0){
		float erem = abs(energy)/ecost;
		value = value * erem;
		energy = 0;
	}

	//Add to the ships inertia.	
	accel.x += orient.y*value*dt/5;
	accel.y += -orient.x*value*dt/5;

	//printf("Accel.x  : %8.3f\n",accel.x);
	//printf("Accel.y  : %8.3f\n",accel.y);
	//printf("Accel.abs: %8.3f\n",vect_abs(&accel));
}

void Ship::Brake(float value, float ticks){
	float dt = ticks/ticks_normal;
	float ecost = abs(value)*dt;
	
	energy -= value*dt;
	if (energy < 0){
		float erem = abs(energy)/ecost;
		value = value * erem;
		energy = 0;
	}

	//Add to the ships inertia.
	accel.x -= orient.x*value*dt/5;
	accel.y -= orient.y*value*dt/5;
	/*
	accel.x -= (t.x*dt)/100.0f;
	accel.y -= (t.y*dt)/100.0f;
	*/
}

void Ship::Print(){
	printf("Ship Texture: %i\n",textureid);
}


void Player::DoPhysics(float ticks){
	ship.DoPhysics(ticks);
}

Player::Player(){

}

Player::~Player(){

}


void Player::Render(void){
	vect2 relative;
	relative.x = 0;
	relative.y = 0;
	ship.Render(relative,true);
}

void Player::Render(vect2 relative, bool center){
	ship.Render(relative,center);
}


void Player::ParseData(uint8_t* data){
	//Data is assumed to be valid, apply it.

	memcpy(&ship.position,data,sizeof(ship.position));
	data+= sizeof(ship.position);

	memcpy(&ship.orient,data,sizeof(ship.orient));
	data+= sizeof(ship.orient);

	memcpy(&ship.momentum,data,sizeof(ship.momentum));
	data+= sizeof(ship.momentum);

	memcpy(&ship.accel,data,sizeof(ship.accel));
	data+= sizeof(ship.accel);

	memcpy(&ship.angular_momentum,data,sizeof(ship.angular_momentum));
	data+= sizeof(ship.angular_momentum);

	memcpy(&ship.angledeg,data,sizeof(ship.angledeg));
	data+= sizeof(ship.angledeg);
		
	int ret = ship.weapon.ParseData(data);
	data += ret;



	return;
	
}

int Asteroid::ParseData(uint8_t* data){
	int len = 0;

	memcpy(&position,data,sizeof(position));
	len += sizeof(position);
	data+= sizeof(position);

	memcpy(&momentum,data,sizeof(momentum));
	len += sizeof(momentum);
	data+= sizeof(momentum);

	memcpy(&rotation,data,sizeof(rotation));
	len += sizeof(rotation);
	data+= sizeof(rotation);

	memcpy(&mass,data,sizeof(mass));
	len += sizeof(mass);
	data+= sizeof(mass);

	memcpy(&size,data,sizeof(size));
	len += sizeof(size);
	data+= sizeof(size);

	memcpy(&alive,data,sizeof(alive));
	len += sizeof(alive);
	data+= sizeof(alive);
	
	return len;
}


int Asteroid::SendData(uint8_t* data){
	int len = 0;

	memcpy(data,&position,sizeof(position));
	len += sizeof(position);
	data+= sizeof(position);

	memcpy(data,&momentum,sizeof(momentum));
	len += sizeof(momentum);
	data+= sizeof(momentum);

	memcpy(data,&rotation,sizeof(rotation));
	len += sizeof(rotation);
	data+= sizeof(rotation);

	memcpy(data,&mass,sizeof(mass));
	len += sizeof(mass);
	data+= sizeof(mass);

	memcpy(data,&size,sizeof(size));
	len += sizeof(size);
	data+= sizeof(size);

	memcpy(data,&alive,sizeof(alive));
	len += sizeof(alive);
	data+= sizeof(alive);
	
	return len;
}

int Player::SendData(uint8_t* data){
	int len = 0;
	

	memcpy(data,&ship.position,sizeof(ship.position));
	len += sizeof(ship.position);
	data+= sizeof(ship.position);	

	memcpy(data,&ship.orient,sizeof(ship.orient));
	len += sizeof(ship.orient);
	data+= sizeof(ship.orient);

	memcpy(data,&ship.momentum,sizeof(ship.momentum));
	len += sizeof(ship.momentum);
	data+= sizeof(ship.momentum);

	memcpy(data,&ship.accel,sizeof(ship.accel));
	len += sizeof(ship.accel);
	data+= sizeof(ship.accel);

	memcpy(data,&ship.angular_momentum,sizeof(ship.angular_momentum));
	len += sizeof(ship.angular_momentum);
	data+= sizeof(ship.angular_momentum);

	memcpy(data,&ship.angledeg,sizeof(ship.angledeg));
	len += sizeof(ship.angledeg);
	data+= sizeof(ship.angledeg);
	
	int ret = ship.weapon.SendData(data);
	data += ret;
	len  += ret;


	return len;
}



void Player::Print(){
	//printf("ship.position.x: %8.3f\n",ship.position.x);
	//printf("ship.position.y: %8.3f\n",ship.position.y);
}


Backdrop::Backdrop(){
	//Generate texture
	LoadFileBitmap("bitmaps/star.bmp",bmpdata, BMP_SIZE_128);

	glGenTextures(1, &textureid);
	glBindTexture(GL_TEXTURE_2D, textureid);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);

	

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

}

Backdrop::~Backdrop(){

}

void Backdrop::Print(){
	printf("Star Texture: %i\n",textureid);
}

void Backdrop::Generate(void){
	int number_of_groups = 100;
	int stars_per_group  = 20;

	int group_radius = 50; 
	int group_gravity = 2; 

	

	float distance; 
    float angle;    

	for(int r=0; r<number_of_groups; r++){ 
		group[r].x = (rand()%10000-5000)/10.0f;
	    group[r].y = (rand()%10000-5000)/10.0f; 
		group[r].z = ((rand()%2000) - 2000)/300.0f;
		for (int i=0; i<stars_per_group; i++){	  		
	  		distance = powf((rand()%3000)/1000.0f,group_gravity) * group_radius;
			angle = ( rand()%360 / 360.0f)*2*3.1415926535;
	  		
	  		star[i+(r*stars_per_group)].x = group[r].x - (sinf(angle) * distance); 
    		star[i+(r*stars_per_group)].y = group[r].y + (cosf(angle) * distance); 
			star[i+(r*stars_per_group)].z = group[r].z - (sinf(angle) * 0.01f * distance); 
		}
	}
	/*
	for (int i =0;i<1000;i++){
		star[i].x = (rand()%10000-5000)/20.0f;
		star[i].y = (rand()%10000-5000)/20.0f;
		//Size
		star[i].z = rand()%7 + -5;
	}
	*/
}

void Backdrop::Move(vect2 value){
	position.x = -value.x;
	position.y = -value.y;
}

void Backdrop::Render(vect2 relative){
	glPushMatrix();
	
	glScalef(0.03f,0.03f,1);
	glTranslatef(-relative.x,-relative.y,-4.5f);

	//Star texture.
	glBindTexture(GL_TEXTURE_2D, textureid);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
		
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
	
	glEnable(GL_DEPTH_TEST);
	
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	


	glBindTexture(GL_TEXTURE_2D, textureid);
	

	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
	for (int i =0;i<1000;i++){	
		 glTexCoord2f(0,0);
		 glVertex3f(-1+star[i].x, -1+star[i].y, star[i].z);
		 glTexCoord2f(1.0, 0);
		 glVertex3f(1.0+star[i].x, -1+star[i].y, star[i].z);
		 glTexCoord2f(1.0, 1.0);
		 glVertex3f(1.0+star[i].x, 1.0+star[i].y, star[i].z);
		 glTexCoord2f(0, 1.0);
		 glVertex3f(-1+star[i].x, 1.0+star[i].y, star[i].z);
	}
	glEnd();




	glDisable(GL_TEXTURE_2D);
	
	


	glPopMatrix();
}


Weapon::Weapon(){
	bullet_cnt = 0;

	position.x = 0;
	position.y = 0;
}


Weapon::~Weapon(){

}

int Weapon::SendData(uint8_t* data){
	int len = 0;	
	//Weapon position
	memcpy(data,&position,sizeof(position));
	data += sizeof(position);
	len += sizeof(position);
	//Send bullet info. 10 bullets.
	for(int i=0;i<MAX_BULLETS;i++){
		memcpy(data,&bullet[i].position,sizeof(bullet[i].position));
		data += sizeof(bullet[i].position);
		len += sizeof(bullet[i].position);
		memcpy(data,&bullet[i].velocity,sizeof(bullet[i].velocity));
		data += sizeof(bullet[i].velocity);
		len += sizeof(bullet[i].velocity);
		memcpy(data,&bullet[i].alive,sizeof(bullet[i].alive));
		data += sizeof(bullet[i].alive);
		len += sizeof(bullet[i].alive);
	}
	return len;
}

//Do the opposite of send data.
int Weapon::ParseData(uint8_t* data){
	int len = 0;	
	//Weapon position
	memcpy(&position,data,sizeof(position));
	data += sizeof(position);
	len += sizeof(position);
	//Send bullet info. 10 bullets.
	for(int i=0;i<MAX_BULLETS;i++){
		memcpy(&bullet[i].position,data,sizeof(bullet[i].position));
		data += sizeof(bullet[i].position);
		len += sizeof(bullet[i].position);
		memcpy(&bullet[i].velocity,data,sizeof(bullet[i].velocity));
		data += sizeof(bullet[i].velocity);
		len += sizeof(bullet[i].velocity);
		memcpy(&bullet[i].alive,data,sizeof(bullet[i].alive));
		data += sizeof(bullet[i].alive);
		len += sizeof(bullet[i].alive);
	}
	return len;
}	

void Weapon::Fire(vect2 vel,float impulse){
	
	bullet[bullet_cnt].Fire(vel,position,orient,impulse);

	bullet_cnt++;
	if (bullet_cnt == MAX_BULLETS){
		bullet_cnt = 0;
	}
}

void Weapon::DoPhysics(float ticks){
	//Do physics on each bullet.
	for (int i =0;i<MAX_BULLETS;i++){
		bullet[i].DoPhysics(ticks);
	}
}




Bullet::Bullet(){
	alive = false;
	ticks_normal = 20000;

	//Position in x,y.
	position.x = 0;
	position.y = 0;

	orient.x = 1;
	orient.y = 0;

	velocity.x = 0;
	velocity.y = 0;

	fade = 1;

	//Generate texture
	LoadFileBitmap("bitmaps/star.bmp",bmpdata, BMP_SIZE_128);

	glGenTextures(1, &textureid);	
	glBindTexture(GL_TEXTURE_2D, textureid);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);

	

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
}

Bullet::~Bullet(){

}

void Bullet::DoPhysics(float ticks){
	float dt = ticks/ticks_normal;
	position.x += dt*velocity.x*0.04f;
	position.y += dt*velocity.y*0.04f;

	fade -= (dt * 0.01f);
	if (fade < 0){
		alive = false;
	}

}

void Bullet::SetPosition(vect2 pos){
	position.x = pos.x;
	position.y = pos.y;
}

void Weapon::Render(vect2 relative){
	for (int i =0;i<MAX_BULLETS;i++){
		bullet[i].Render(relative);
	}
}


void Bullet::Fire(vect2 vel, vect2 pos, vect2 ori, float impulse){
	alive = true;
	fade = 1;
	position = pos;
	orient = ori;

	velocity.x = (orient.x * impulse)+vel.x;
	velocity.y = (orient.y * impulse)+vel.y;


}

void Bullet::Render(vect2 relative){
	if (!alive)
		return;
	glPushMatrix();
	glTranslatef(position.x-relative.x,position.y-relative.y,0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);

	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
	
	
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, textureid);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float rot = rand()%360;
	glRotatef(rot,0.f,0.f,1.0f);

	glScalef(0.5f,0.5f,1);
	glRotatef(rand()%360,0.f,0.f,1.0f);

	glBegin(GL_QUADS);		
		glColor4f(1,1,1,0.5f*fade);	
	
		glTexCoord2f(0,0);
		glVertex3f(-1, -1, 0);
		glTexCoord2f(1.0, 0);
		glVertex3f(1.0, -1, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, 0);
		glTexCoord2f(0, 1.0);
		glVertex3f(-1, 1.0, 0);
	glEnd();

	glScalef(0.5f,0.5f,1);

	glBegin(GL_QUADS);		
		glColor4f(1,1,0,0.5f*fade);	
	
		glTexCoord2f(0,0);
		glVertex3f(-1, -1, 0);
		glTexCoord2f(1.0, 0);
		glVertex3f(1.0, -1, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, 0);
		glTexCoord2f(0, 1.0);
		glVertex3f(-1, 1.0, 0);
	glEnd();
	glScalef(0.5f,0.25f,1*fade);
	glRotatef(rand()%360,0.f,0.f,1.0f);
	glColor4f(1,0,0,0.6f);	
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(-1, -1, 0);
		glTexCoord2f(1.0, 0);
		glVertex3f(1.0, -1, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 1.0, 0);
		glTexCoord2f(0, 1.0);
		glVertex3f(-1, 1.0, 0);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}



bool isinrange(vect2 a, vect2 b, float range_a, float range_b){
	

	if (a.x-range_a < b.x+range_b){
		if (a.x+range_a > b.x - range_b){
			if (a.y-range_a < b.y+range_b){
				if (a.y+range_a > b.y - range_b){
					/*printf("a.x: %8.3f\n",a.x);
					printf("a.y: %8.3f\n",a.y);
					printf("b.x: %8.3f\n",b.x);
					printf("b.y: %8.3f\n",b.y);
					printf("Collision!\n");*/
					return true;
				}
			}
		}
	}
	return false;
}



bool CheckCollision(Bullet* bullet,Ship* ship){
	if (bullet->alive == false){
		return false;
	}


	if (isinrange(bullet->position,ship->position,.5,.5)){
		return true;
	}
	return false;
}

bool CheckCollision_Object(Bullet* bullet,Object* object){
	if (bullet->alive == false){
		return false;
	}
	if (object->alive == false){
		return false;
	}

	if (isinrange(bullet->position,object->position,.25,object->size/1.8)){
		return true;
	}
	return false;
}

bool CheckCollision_ShipObject(Ship* a,Object* b){
	
	if (b->alive == false){
		return false;
	}

	if (isinrange(a->position,b->position,.5,b->size/1.8)){
		return true;
	}
	return false;
}

bool CheckCollision_ObjectObject(Object* a,Object* b){
	if (a->alive == false){
		return false;
	}
	if (b->alive == false){
		return false;
	}

	if (isinrange(a->position,b->position,a->size/1.8,b->size/1.8)){
		return true;
	}
	return false;
}



bool CheckCollision_AstAst(Asteroid* a,Asteroid* b){
	if (a->alive == false){
		return false;
	}
	if (b->alive == false){
		return false;
	}

	if ((a->collision_timout  > 0) || (b->collision_timout  > 0) ){
		return false;
	}

	if (isinrange(a->position,b->position,a->size/1.8,b->size/1.8)){
		return true;
	}
	return false;
}

GLuint Crate::crate_texture[2];
uint8_t Crate::crate_texture_data[2][BMP_SIZE_128];


void Crate::init(){
	//Generate texture
	LoadFileBitmap("bitmaps/crate.bmp",crate_texture_data[0], BMP_SIZE_128);
	LoadFileBitmap("bitmaps/crate_metal.bmp",crate_texture_data[1], BMP_SIZE_128);

	glGenTextures(2, &crate_texture[0]);

	glBindTexture(GL_TEXTURE_2D, crate_texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, crate_texture_data[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

	
	glBindTexture(GL_TEXTURE_2D, crate_texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, crate_texture_data[1]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
}

void Crate::PreRender(){
	
	glBindTexture(GL_TEXTURE_2D, crate_texture[0]);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, crate_texture_data[0]);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering*/

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
		
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	

	glDisable(GL_DEPTH_TEST);
}


Crate::Crate(){
	size = 0.3f;

	rotation = 2;
	angledeg = 0;

	alive = true;
	health = 80;
	health_max = 80;

	mass = 10;
}

Crate::~Crate(){

}

bool Object::GetFreeSmoke(Smoke** s){
	for(int i =0;i<max_smoke;i++){
		if (smoke[i].alive == false){
			*s = &smoke[i];
			return true;
		}
	}
	return false;
}

bool Asteroid::GetFreeCrate(Crate** c){
	for(int i =0;i<max_crate;i++){
		if (crate[i].alive == false){
			*c = &crate[i];
			return true;
		}
	}
	return false;
}


void Crate::Destroy(){
	Smoke* s;
	int num_smoke = 20;
	for(int i =0;i<num_smoke;i++){
		if (!GetFreeSmoke(&s)){
			//Out of smoke.
			break;
		}
		s->SetPosition(position);
		vect2 smokevel = velocity;

		smokevel.x += (rand()%20/10.0f - 1)/1.0f;
		smokevel.y += (rand()%20/10.0f - 1)/1.0f;		
		s->color.r = 1;
		s->color.g = 1;
		s->color.b = 1;
		s->SetVelocity(smokevel);
		s->fade = 1;
		s->alive = true;		
	}
	alive = false;
}






void Crate::Render(vect2 relative){	

	
	if (alive){	
		glPushMatrix();

		glTranslatef(position.x-relative.x,position.y-relative.y,zoom);
	
	
		glScalef(size,size,1.0f);
		
		glRotatef(angledeg,0.f,0.f,1.0f);

		glEnable(GL_TEXTURE_2D);

		glColor4f(1,1,1,1);
		glBegin(GL_QUADS);
			 glTexCoord2f(0,0);
			 glVertex3f(-1, -1, -0.0f);
			 glTexCoord2f(1.0, 0);
			 glVertex3f(1.0, -1, -0.0f);
			 glTexCoord2f(1.0, 1.0);
			 glVertex3f(1.0, 1.0, -0.0f);
			 glTexCoord2f(0, 1.0);
			 glVertex3f(-1, 1.0, -0.0f);
		glEnd();

		glRotatef(-angledeg,0.f,0.f,1.0f);
		//Render stats.
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);	
			//Bar base.
			glColor4f(1,1,1,0.2f);
			glVertex3f(-1, 1, -0.0f);	
			glVertex3f(1.0, 1, -0.0f);	
			glVertex3f(1.0, 1.1, -0.0f);	
			glVertex3f(-1, 1.1, -0.0f);
			//Hull value.
			glColor4f(1,1,0,0.5f);
			glVertex3f(-1, 1, -0.0f);
			glVertex3f(((health*2.0f)/health_max)-1.0f, 1, -0.0f);	
			glVertex3f(((health*2.0f)/health_max)-1.0f, 1.05, -0.0f);	
			glVertex3f(-1, 1.05, -0.0f);
	
		glEnd();


		glDisable(GL_TEXTURE_2D);
		//glEnable(GL_DEPTH_TEST);



		glPopMatrix();
	}
	
}

GLuint Asteroid::asteroid_texture;
uint8_t Asteroid::asteroid_texture_data[TGA_SIZE_256];	

int Asteroid::num_asteroids;
Asteroid* Asteroid::asteroids;

void Asteroid::init(Asteroid* ast, int num){
	//Generate texture
	LoadFileTGA("bitmaps/rock2.tga",asteroid_texture_data, TGA_SIZE_256);

	glGenTextures(1, &asteroid_texture);
	glBindTexture(GL_TEXTURE_2D, asteroid_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, asteroid_texture_data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

    asteroids = ast;
    num_asteroids = num;
}

void Asteroid::PreRender(){
	glBindTexture(GL_TEXTURE_2D, asteroid_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, asteroid_texture_data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
		
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	

	glDisable(GL_DEPTH_TEST);
}


//Creates asteroids 
void Asteroid::CreateAsteroids(int num, float size_max, vect2 pos, vect2 vel){
	int cnt = num;
	float new_size;

	for(int i=0;i<num_asteroids;i++){
		if (cnt ==0){
			return;
		}
		if (!(asteroids[i].alive)){
			cnt--;

			new_size = (((rand()%50)+50)/100.0f) * size_max;

			vect2 ast_vel = vel;
			ast_vel.x += ((rand()%20/10.0f - 1)/1.0f);
			ast_vel.y += ((rand()%20/10.0f - 1)/1.0f);
			
			vect2 ast_pos = pos;

			ast_pos.x += (((rand()%20/10.0f) - 1)/1.0f);
			ast_pos.y += (((rand()%20/10.0f) - 1)/1.0f);


			asteroids[i].SetSize(new_size);	
			asteroids[i].SetPosition(ast_pos);
			asteroids[i].SetVelocity(ast_vel);

			//asteroids[i].CleanSmoke();

			asteroids[i].collision_timout = 1000000;
			asteroids[i].fade  = 1;
			asteroids[i].alive = true;

		}
	}
}


Asteroid::Asteroid(){
	position.x = 0;
	position.y = 0;

	ticks_normal = 20000;

	rotation = rand()%100/50.0f;
	angledeg = 0;

	health = 20 + rand()%20;

	momentum.x = (rand()%100/50.0f -1)*4.0f;
	momentum.y = (rand()%100/50.0f -1)*4.0f;

	size = ((rand()%10000) + 2500)/5000;

	mass = size*5;

	fade = 1;

	collision_timout = 100000;

	alive = true;
}


Asteroid::~Asteroid(){

}





void Asteroid::Render(vect2 relative){	
	if (alive){
		glPushMatrix();

		glTranslatef(position.x-relative.x,position.y-relative.y,0);
		glRotatef(angledeg,0,0,1.0f);
		glScalef(size,size,1);

		glBegin(GL_QUADS);
			glColor4f(1,1,1,fade);
			glTexCoord2f(0,0);		glVertex3f(-1, -1, 0.0f);
			glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
			glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
			glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);	
		glEnd();

		glPopMatrix();
	}
}


void Object::RenderSmoke(vect2 relative){	
	
}

void Crate::DoPhysics(float ticks){

	float dt = ticks/ticks_normal;
	
	angledeg += (rotation * dt);
	anglerad = (angledeg / 360.0f)*2*3.1415926535;

	velocity.x = momentum.x / mass;
	velocity.y = momentum.y / mass;

	position.x += dt*velocity.x*0.04f;
	position.y += dt*velocity.y*0.04f;

}


void Ship::DoPhysics(float ticks){
	if (!alive){
		return;
	}
	//Weapon
	weapon.position = position;
	weapon.orient = orient;
	weapon.DoPhysics(ticks);


	float dt = ticks/ticks_normal;
	float df = friction_normal/friction;

	//Fricion on acceleration
	accel.x = accel.x * df;
	accel.y = accel.y * df;

	momentum.x += (accel.x * 5)/dt;
	momentum.y += (accel.y * 5)/dt;

	velocity.x = momentum.x / mass;
	velocity.y = momentum.y / mass;


	

	//Do all local physics.
	//inertia.x = friction.x;
	//inertia.y = friction.y;

	RotateDeg(angular_momentum,ticks);

	angular_momentum = angular_momentum/(powf(1.1f,dt));

	position.x += dt*velocity.x*0.04f;
	position.y += dt*velocity.y*0.04f;

	//Recharge the shields at a fixed rate.
	if (shield_enabled == false){		
		shield += (dt*shield_regen);
		if (shield_auto_enable){
			if ((shield/shield_max) > 0.5f){
				shield_enabled = true;
			}
		}
	}else if (shield_enabled == true){
		shield += (dt*0.25*shield_regen);
	}
	if (shield > shield_max){
		shield = shield_max;
	}

	//Recharge Health
	health += (dt*health_regen);		
	if (health > health_max){
		health = health_max;
	}

	//Recharge energy at a fixed rate.
	energy += (dt*energy_regen);
	if (energy > energy_max){
		energy = energy_max;
	}

	//Shield hit animation
	anim_shield_washit -= anim_shield_washit_fact;
	if (anim_shield_washit < 0){
		anim_shield_washit = 0;
	}

	//printf("Player: %6.3f %6.3f %6.3f %6.3f\n",angledeg,anglerad,accel.x,accel.y);
}


//Regen the thields by using up energy.
void Ship::RegenShields(float value, float ticks){	
	float dt = ticks/ticks_normal;
	float rate = (dt*shield_regen*value);
	energy -= rate;
	if (energy < 0){
		rate += energy;
		energy = 0;
	}
	rate /= 4;


	if (shield_enabled == false){		
		shield += rate;
		if (shield_auto_enable){
			if ((shield/shield_max) > 0.5f){
				shield_enabled = true;
			}
		}
	}else if (shield_enabled == true){
		shield += rate/4;
	}
	if (shield > shield_max){
		shield = shield_max;
	}

}


void Asteroid::DoPhysics(float ticks){
	if (!alive){
		return;
	}


	collision_timout -= ticks;
	if (collision_timout < 0){
		collision_timout = 0;
	}

	float dt = ticks/ticks_normal;
	
	angledeg += (rotation * dt);
	anglerad = (angledeg / 360.0f)*2*3.1415926535;

	velocity.x = momentum.x / mass;
	velocity.y = momentum.y / mass;

	if (vect_abs_sqare(&velocity) > 100){
		momentum.x = momentum.x / 2;
		momentum.y = momentum.y / 2;
	}

	position.x += dt*velocity.x*0.04f;
	position.y += dt*velocity.y*0.04f;

	if (size < 0.2f){
		fade -= dt*0.01;
		if (fade < 0){
			alive = false;
		}
	}
		
}


void Object::DoDamage(float damage, vect2 impact,vect2 pos){
	//Get some velocity.
	momentum.x += impact.x;
	momentum.y += impact.y;

	health -= damage;
	if (health < 0){
		health = 0;		
		Destroy();
	}

	DoClassDamage(damage,impact,pos);
}

void Crate::DoClassDamage(float damage, vect2 impact,vect2 pos){
	
}


//Gets the next free asteroid.
bool Asteroid::GetFreeAsteroid(Asteroid** a){
	for(int i=0;i<num_asteroids;i++){		
		if (asteroids[i].alive == false){
			*a = &asteroids[i];
			return true;
		}
	}
	return false;
}

/*
	Does specific damage, crates new asteroid fragments along axis of impact.
*/
void Asteroid::DoClassDamage(float damage, vect2 impact,vect2 pos){
	size -= 0.1;
	Asteroid* a;
	float size_max = 0.5f;

	//Spawn new asteroids.
	if (size > 0.5){
		int new_ast = size * 3;
		while(new_ast--){
			if (GetFreeAsteroid(&a)){
				float new_size = (((rand()%50)+50)/100.0f) * size_max;

				vect2 ast_vel = vect_unit(&impact);
				ast_vel.x = -ast_vel.x;
				ast_vel.y = -ast_vel.y;
				ast_vel.x += ((rand()%20/10.0f - 1)/5.0f);
				ast_vel.y += ((rand()%20/10.0f - 1)/5.0f);
				
				vect2 ast_pos = pos;
				ast_pos.x += (((rand()%20/10.0f) - 1)/5.0f);
				ast_pos.y += (((rand()%20/10.0f) - 1)/5.0f);


				a->SetSize(new_size);	
				a->SetPosition(ast_pos);
				a->SetVelocity(ast_vel);

				a->collision_timout = 1000000;
				a->fade  = 1;
				a->alive = true;
			}
		}
		//CreateAsteroids(new_ast,.5f,pos,velocity);
	}
}




void Asteroid::Destroy(){
	//Create smoke.
	int num_smoke = 30 * size;
	int num_fire = 50 * size;
	if (num_smoke > 96){
		num_smoke = 96;
	}

	//Play sound
	#if COMPILE_SOUND
	sound_play(SND_IMPACT,10.0f);
	#endif

	int total_decl = num_smoke + num_fire;
	
	Smoke* s;

	while(total_decl--){
		if (!GetFreeSmoke(&s)){
			break;
		}
		s->SetPosition(position);
		vect2 smokevel = velocity;	
		smokevel.x += ((rand()%20/10.0f - 1)/1.0f)*size;
		smokevel.y += ((rand()%20/10.0f - 1)/1.0f)*size;
		s->SetVelocity(smokevel);
		
		s->alive = true;

		if (num_fire--){
			//Color
			s->color.b = 0;
			float r =rand()%100/100.0f;
			float g =rand()%100/100.0f;
			s->color.g = g;
			s->color.r = 1;
			s->fade = .5;
			s->fade_speed = ((rand()%10) + 5)/500.0f;
		}else{
			s->fade = .8;
			s->color.r = 1;
			s->color.g = 1;
			s->color.b = 1;
			s->fade_speed = ((rand()%10) + 10)/1000.0f;
		}
	}

	if (size < 1){
		int r = rand()%10;
		if (r == 5){
			//Make a crate.
			Crate* c;
			if(GetFreeCrate(&c)){
				c->SetPosition(position);
				vect2 vel = velocity;
				vel.x /= 3;
				vel.y /= 3;
				c->SetVelocity(vel);
				
				float sz = size/1.8f;
				if (sz < .2) {
					sz = .2;
				}
				c->size = sz;
				c->health_max = (((rand()%50)*sz) + 50);
				c->health = (c->health_max) - (sz*(rand()%30));
				c->alive = true;
			}
		}
	}


	//Spawn new asteroids.
	if (size > 0.5){
		int new_ast = size * 3;

		CreateAsteroids(new_ast,size/2,position,velocity);
	}
	

	alive = false;
}

GLuint Smoke::smoke_texture;
uint8_t Smoke::smoke_texture_data[BMP_SIZE_256];	

int Asteroid::max_smoke;
Smoke* Asteroid::smoke;

void Asteroid::initSmoke(Smoke* smokes, int max){
	max_smoke = max;
	smoke = smokes;
}

int Asteroid::max_crate;
Crate* Asteroid::crate;

void Asteroid::initCrate(Crate* crates, int max){
	max_crate = max;
	crate = crates;
}

void Smoke::init(){
	
	//Generate texture
	LoadFileBitmap("bitmaps/smoke2.bmp",smoke_texture_data, BMP_SIZE_256);

	glGenTextures(1, &smoke_texture);
	glBindTexture(GL_TEXTURE_2D, smoke_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_BGR, GL_UNSIGNED_BYTE, smoke_texture_data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
}

Smoke::Smoke(){
	rotation = (rand()%100/25.0f) - 2;
	angledeg = rand()%360;	

   	position.x = 0;
	position.y = 0;  

	
	fade = rand()%25 + 25;
	fade_speed = ((rand()%10) + 5)/200.0f;

	size = rand()%1000/2000.0f + 0.2f;

	alive = false;

	color.r = 1;
	color.g = 1;
	color.b = 1;

}




Smoke::~Smoke(){
	
}

void Smoke::PreRender(void){
	glBindTexture(GL_TEXTURE_2D, smoke_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_BGR, GL_UNSIGNED_BYTE, smoke_texture_data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_DEPTH_TEST);
}

void Smoke::Render(vect2 relative){	
	if (alive){
		glPushMatrix();

		glTranslatef(position.x-relative.x,position.y-relative.y,0);

		glRotatef(angledeg,0,0,1.0f);	
	
		glScalef(size,size,1);

		//glColor4f(1,1,1,1);
		glColor4f(color.r,color.g,color.b,fade);
		//glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);		glVertex3f(-1, -1, 0.0f);
			glTexCoord2f(1.0, 0);	glVertex3f(1.0, -1, 0.0f);
			glTexCoord2f(1.0, 1.0);glVertex3f(1.0, 1.0, 0.0f);
			glTexCoord2f(0, 1.0);	glVertex3f(-1, 1.0, 0.0f);
		glEnd();

 		glPopMatrix();
	}

}

void Smoke::DoPhysics(float ticks){
	if (!(alive)){
		return;
	}
	float dt = ticks/ticks_normal;
	
	angledeg += (rotation * dt);
	anglerad = (angledeg / 360.0f)*2*3.1415926535;

	position.x += dt*velocity.x*0.04f;
	position.y += dt*velocity.y*0.04f;

	//Smoke specific.
	fade -= dt*fade_speed;
	if (fade < 0){
		fade = 0;
		alive = false;		
	}
}

Object::Object(){
	ticks_normal = 20000;

	position.x = 0;
	position.y = 0;

	velocity.x = 0;
	velocity.y = 0;
	
	

	alive = true;

	size = 1;
	mass = 1;
}


void Object::SetPosition(vect2 pos){
	position = pos;
}

void Object::SetVelocity(vect2 vel){
	velocity = vel;

	momentum.x  = velocity.x * mass;
	momentum.y  = velocity.y * mass;
}

void Object::SetSize(float s){
	size = s;
	mass = size*5;
	health = size * 12;
}