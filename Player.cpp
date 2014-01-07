#include "Player.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "asteroids_common.h"

#if COMPILE_SOUND
extern void sound_play(int,int);
#endif

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

//???
float whatvect_abs(vect2* a, vect2* b)
{
	float t;
	t = sqrtf((a->x*a->x) + (b->x*b->x));
	return t;
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