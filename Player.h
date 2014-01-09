#ifndef PLAYER_H
#define PLAYER_H

#include <gl/glew.h>
#include <gl/glu.h>		
#include <math.h>

#include "Types.h"

#define BMP_SIZE_128 128*128*3
#define BMP_SIZE_256 256*256*3
#define BMP_SIZE_1024 1024*1024*3
#define TGA_SIZE_256 256*256*4

#define PI 3.14159265f


/*
Player has a ship.
*/
class Weapon;
class Bullet;

class Player;
class Ship;


class Object;
class Smoke;
class Asteroid;


class Obj;

class Obj{

public:
	Obj();
	~Obj();
	void init(vect2* pos, float* rot, float* size);
	void doObjPhysics(float ticks);
	bool checkCollision(Obj* obj);

	//Pinters to global memory.
	vect2* position;
	float* rotation;
	float* size;


	float mass;
	vect2 momentum;
	vect2 velocity;
};


/* 
	Manages objects... duh.

	Basically handles the loading of everything, textures, locations etc.

	Every object's coordinates, orientation and size are stored in a few VBOs.
	Since evety object is drawn as a sprite on a sqare, they all have a -1 to +1 sqare with a 0-1 texture.

	They're drawn as quads, so each vertex needs coordinate data, orientation data.

*/
class ObjManager{
public:
	ObjManager();
	~ObjManager();

	void init();
	void init_textures();
	void init_shaders();

	GLuint make_shader(GLenum type, char *filename);
	GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

	
	#define MAX_NUM_OBJ 500

	//Always the same square.
	GLfloat g_vertex_data[8*MAX_NUM_OBJ];


	GLfloat g_vertex_location[8*MAX_NUM_OBJ];	

	GLfloat g_vertex_size[4*MAX_NUM_OBJ];
	GLfloat g_vertex_rotation[4*MAX_NUM_OBJ];

	//An array of indices... which are also the same.
	//GLushort g_element_data[6*1000];
	

	//Object ofsetts.
	//GLfloat g_offset_data[8];

	int num_balls;
	GLfloat b_position[2*MAX_NUM_OBJ];
	//GLfloat b_rotation[5000];
	//GLfloat b_size[5000];

	//And the buffers that go with it.
	GLuint vertex_buffer;
	GLuint rotation_buffer;
	GLuint location_buffer;
	GLuint size_buffer;
	
	GLfloat color4[4];

	float orientation;
	float zoom;

	//Shader arguments
	GLint va_position;
	GLint va_location;
	GLint va_rotation;	
	GLint va_color;
	GLint va_zoom;

	GLuint va_scale;

	GLuint va_textures[5];


	void render();
	void _render();
	void doPhysics(float);

	void Rotate(float,float);

	//Compiled Shaders
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;

	
	//Textures
	int num_textures;
	GLuint textures[2];
	uint8_t texture_data_rock[TGA_SIZE_256];
	uint8_t texture_data_ball[TGA_SIZE_256];
	uint8_t texture_data_back_1[BMP_SIZE_1024];

	GLfloat position[6];

	Obj object[MAX_NUM_OBJ];

private:
	
};

class Object{
public:
	Object();
	virtual void Render(vect2){};
	
	virtual void DoPhysics(float ticks){};

	void DoDamage(float damage, vect2 impact, vect2 pos);
	virtual void DoClassDamage(float damage, vect2 impact, vect2 pos){};

	virtual void Destroy(void){};

	static bool GetFreeSmoke(Smoke** smoke);

	void RenderSmoke(vect2);
	void SetPosition(vect2);
	void SetVelocity(vect2);
	void SetSize(float);

	//Variables
	vect2 position;

	vect2 velocity;
	vect2 momentum;
	
	float zoom;
	float ticks_normal;

	float rotation;
	float angledeg;
	float anglerad;

	bool alive;

	float health;
	float health_max;

	float size;
	float mass;
protected:
	
	


	//Should be able to make smoke.
	static int max_smoke;
	static Smoke* smoke;
	
};


class Crate: public Object{
public:
	Crate();
	~Crate();

	static void init();
	static void PreRender(void);
	
	void Render(vect2 ref);
	void DoPhysics(float ticks);

	void DoClassDamage(float damage, vect2 impact, vect2 pos);
	void Destroy(void);
	
private:	

	static GLuint crate_texture[2];
	static uint8_t crate_texture_data[2][BMP_SIZE_128];
};

//Smoke* GetFreeSmoke(int max, Smoke* smoke);

class Smoke : public Object{
public:
	static void init();
	static void PreRender(void);

	

	Smoke();	
	~Smoke();
	void Render(vect2);
	void DoPhysics(float ticks);
	float fade;
	float fade_speed;

	color_f color;
private:
	
	
	
	static GLuint smoke_texture;
	static uint8_t smoke_texture_data[BMP_SIZE_256];	
};


class Asteroid :public Object{
public:
	static void init(Asteroid*,int);
	static void initSmoke(Smoke* smokes, int max);
	static void initCrate(Crate* crates, int max);
	static void PreRender(void);

	static void CreateAsteroids(int num, float size_max, vect2 pos, vect2 mom);
	

	Asteroid();
	~Asteroid();

	void Render(vect2);
	
	int SendData(uint8_t* data);
	int ParseData(uint8_t* data);

	void DoPhysics(float ticks);

	void DoClassDamage(float damage, vect2 impact, vect2 pos);
	void Destroy(void);

	static int num_asteroids;
	
	static int max_crate;
	static Crate* crate;
	static bool GetFreeCrate(Crate** c);
	static bool GetFreeAsteroid(Asteroid** a);

	float collision_timout;

private:
	float fade;

	
	static Asteroid* asteroids;

	static GLuint asteroid_texture;
	static uint8_t asteroid_texture_data[TGA_SIZE_256];	
};





class Bullet{
public:
	Bullet();
	~Bullet();
	void DoPhysics(float ticks);
	void Render(vect2);
	void Fire(vect2 vel, vect2 pos, vect2 ori, float impulse);
	void SetPosition(vect2 pos);
	vect2 orient;
	vect2 position;
	vect2 velocity;
	bool alive;
	float fade;
private:
	
	GLuint textureid;
	uint8_t bmpdata[BMP_SIZE_128];

	float ticks_normal;
};

class Weapon{
public:
	Weapon();
	~Weapon();
	void Fire(vect2 vel, float impulse);
	void DoPhysics(float ticks);
	void Render(vect2);
	int SendData(uint8_t* data);
	int ParseData(uint8_t* data);
	vect2 orient;
	vect2 position;

	uint16_t bullet_cnt;
	#define MAX_BULLETS 10
	Bullet bullet[MAX_BULLETS];
private:
	
};

class Backdrop{
public:
	Backdrop();
	~Backdrop();
	void Render(vect2);
	void Generate();
	void Move(vect2);

	void Print();
	vect2 position;
private:
	GLuint textureid;
	uint8_t bmpdata[BMP_SIZE_128];
	vect3 star[10000];
	vect3 group[200];
};



class Ship{
public:
	Ship();
	~Ship();
	void Render(vect2,bool center);
	void Move(float,float);
	void Strafe(float,float);
	void Brake(float,float);
	void RotateDeg(float,float);
	void RotateImpulse(float impulse, float ticks);
	void DoPhysics(float ticks);
	void Fire(float impulse);
	void LoadShip(char* name);
	void EnableShield(bool);
	void RegenShields(float value,float ticks);

	void DoDamage(float value);

	void Print();

	vect2 velocity;
	vect2 momentum;

	vect2 orient;
	vect2 position;
	vect2 accel;

	float angular_momentum;
	float angledeg;
	color_f color;
	
	bool shield_enabled;
	Weapon weapon;

	float health;
	float health_max;
	float health_regen;

	float shield;
	float shield_max;
	float shield_regen;


	float energy;
	float energy_max;
	float energy_regen;

	bool alive;
private:
	float zoom;

	bool shield_auto_enable;

	float anim_shield_washit;
	float anim_shield_washit_fact;

	vect2 force;
	GLuint textureid;
	uint8_t bmpdata[TGA_SIZE_256];


	GLuint shieldtext[2];
	uint8_t shield_data[2][TGA_SIZE_256];

	GLuint enginetext;
	uint8_t engine_data[BMP_SIZE_256];

	float anglerad;
		
	float mass;
	float friction;
	vect2 inertia;
	float ticks_normal;
	float friction_normal;
};

class Player{
public:
	Player();
	~Player();
	void Render(void);
	void Render(vect2,bool center);
	void DoPhysics(float ticks);
	void ParseData(uint8_t* data);
	int SendData(uint8_t* data);
	void Print();
	Ship ship;
private:
};




bool isinrange(vect2 a, vect2 b, float range_a, float range_b);
void DoDamage(Ship* ship, int damage);

bool CheckCollision(Bullet* bullet,Ship* ship);
bool CheckCollision_Object(Bullet* bullet,Object* object);
bool CheckCollision_ShipObject(Ship* a,Object* b);
bool CheckCollision_ObjectObject(Object* a,Object* b);

bool CheckCollision_AstAst(Asteroid* a,Asteroid* b);

int LoadFileBitmap(char* path, uint8_t* target, uint32_t size);
int LoadFileTGA(char* path, uint8_t* target, uint32_t size);



float vect_abs(vect2* a);
vect2 vect_unit(vect2* a);
vect2 vect_diff(vect2* a, vect2* b);
float vect_ordeg(vect2* a);

#endif
