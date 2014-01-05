//OLD



class Asteroid{
public:
	Asteroid();
	~Asteroid();
	void Render(vect2,bool center);
	void DoPhysics(float ticks);
private:
	vect2 position;

	vect3 orient;

	float angledeg;
	float anglerad;
	float rotation;

	uint32_t num_faces;
	vect3 vface[20];

	uint32_t num_bfaces;
	vect3 vbface[20];

	GLuint textureid;
	uint8_t bmpdata[BMP_SIZE_128];

	float ticks_normal;
};


Asteroid::Asteroid(){
	position.x = 0;
	position.y = 0;

	ticks_normal = 20000;

	rotation = 2;
	angledeg = 0;

	//Generate texture
	LoadFileBitmap("bitmaps/rock1.bmp",bmpdata, BMP_SIZE_128);

	glGenTextures(1, &textureid);
	glBindTexture(GL_TEXTURE_2D, textureid);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);

	

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering

    num_faces = 13;

    vface[0].z = 0.8f;
	

	float step = (360.0f/num_faces);
	float param = 0;
	float scale = 1;


	for (uint8_t i=1;i<=num_faces;i++){			
		param += step;
		scale = (rand()%200)/200.0f + 0.9;


		vface[i].x = scale*sinf(param*PI/180);
		vface[i].y = scale*cosf(param*PI/180);
		vface[i].z = 0.2f;
	}


	num_bfaces = 13;

    vbface[0].z = -0.8f;


	step = (360.0f/num_bfaces);
	param = 0;
	scale = 1;


	for (uint8_t i=1;i<=num_bfaces;i++){			
		param += step;
		scale = (rand()%200)/200.0f + 0.9;


		vbface[i].x = scale*sinf(param*PI/180);
		vbface[i].y = scale*cosf(param*PI/180);

		vbface[i].z = -0.2f;
	}

	


	/*

    vface[1].x = 1;

    vface[2].y = 1;

    vface[3].x = -1;

	vface[4].y = -1;

	vface[5].y = -1;
	vface[5].x = -1;*/
}

Asteroid::~Asteroid(){

}

void Asteroid::Render(vect2 relative,bool center){	
	glPushMatrix();

	//glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	//glLoadIdentity ();	

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear Screen And Depth Buffer
    //glLoadIdentity();                           // Reset The Current Matrix
    //glTranslatef(0.0f,0.0f,-5.0f);                      // Move Into The Screen 5 Units


	//glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
    //glShadeModel(GL_SMOOTH);                        // Enable Smooth Shading
    //glClearColor(0.0f, 0.0f, 0.0f, 0.5f);                   // Black Background
    //glClearDepth(1.0f);                         // Depth Buffer Setup
    //glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    //glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);          // Really Nice Perspective 


	if (center){
		glTranslatef(-relative.x,-relative.y,0);		
	}else{
		glTranslatef(position.x-relative.x,position.y-relative.y,0);
	}

	glRotatef(angledeg,1.0f,1.0f,1.0f);
	
	
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpdata);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);					// Set The Blending Function For Translucency
	glEnable(GL_BLEND);
	

	
	//glCullFace(GL_FRONT_AND_BACK);
	//glDisable(GL_DEPTH_TEST);

	//glBindTexture(GL_TEXTURE_2D, textureid);


	glColor4f(1,1,1,1);
	/*
	glBegin(GL_TRIANGLES);
		 glTexCoord2f(0,0);
		 glVertex3f(-1, -1, -0.1f);
		 glTexCoord2f(1.0, 0);
		 glVertex3f(1.0, -1, -0.1f);
		 glTexCoord2f(1.0, 1.0);
		 glVertex3f(1.0, 1.0, -0.0f);
	glEnd();
	*/


	

	uint8_t texture_swap = 1;	
	glBegin(GL_TRIANGLE_FAN);
		
		glTexCoord2f(0  ,   0);	glVertex3fv((GLfloat*)&vface[0]);
		glTexCoord2f(1.0,   0);	glVertex3fv((GLfloat*)&vface[1]);

		for (int i=2;i<=num_faces;i++){
			if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}
			glVertex3fv((GLfloat*)&vface[i]);		
		}
		if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}

		glVertex3fv((GLfloat*)&vface[1]);
		
	glEnd();
	//Bottom
	
	glBegin(GL_TRIANGLE_FAN);
		
		glTexCoord2f(0  ,   0);	glVertex3fv((GLfloat*)&vbface[0]);
		glTexCoord2f(1.0,   0);	glVertex3fv((GLfloat*)&vbface[1]);

		for (int i=2;i<=num_bfaces;i++){
			if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}
			glVertex3fv((GLfloat*)&vbface[i]);		
		}
		if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}

		glVertex3fv((GLfloat*)&vbface[1]);
		
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
		
		glTexCoord2f(0  ,   0);	glVertex3fv((GLfloat*)&vface[1]);
		glTexCoord2f(1.0,   0);	glVertex3fv((GLfloat*)&vbface[1]);

		for (int i=2;i<=num_bfaces;i++){
			if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}
			glVertex3fv((GLfloat*)&vface[i]);		
			glVertex3fv((GLfloat*)&vbface[i]);	
		}
		if (texture_swap){
				texture_swap = 0;
				glTexCoord2f(1.0, 1.0);
			}else{
				texture_swap = 1;
				glTexCoord2f(1.0,   0);
			}

		glVertex3fv((GLfloat*)&vface[1]);
		
	glEnd();
	


	/*

	glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    // Top Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
glEnd();
*/



	
	//glDisable(GL_TEXTURE_2D);
	//glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

void Asteroid::DoPhysics(float ticks){
	

	float dt = ticks/ticks_normal;
	
	angledeg += (rotation * dt);
	anglerad = (angledeg / 360.0f)*2*3.1415926535;

	//printf("ticks   : %8.3f\n",ticks);
	//printf("Angledeg: %8.3f\n",angledeg);

}