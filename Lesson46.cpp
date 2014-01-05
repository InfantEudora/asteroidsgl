/**************************************
*                                     *
*   Jeff Molofee's Basecode Example   *
*          nehe.gamedev.net           *
*                2001                 *
*                                     *
*    All Code / Tutorial Commenting   *
*       by Jeff Molofee ( NeHe )      *
*                                     *
**************************************/
#include "udpserver.h"
#include <gl/glew.h>											// Header File For The OpenGL32 Library
#include <gl/glu.h>												// Header File For The GLu32 Library
#include "NeHeGL.h"												// Header File For NeHeGL
#include <math.h>												// We'll Need Some Math
#include <stdio.h>
#include "ARB_MULTISAMPLE.h"
#include "NeHeGL.h"		




// ROACH
#include "arb_multisample.h"
bool domulti = true;
bool doangle = true;
bool dodynamiczoom = true;
// ENDROACH

#include "Player.h"
#include "Timer.h"
#include "gl_font.h"
#include "Thread.h"
#include "NetworkManager.h"

extern GLFont g_font;

#pragma comment( lib, "opengl32.lib" )							// Search For OpenGL32.lib While Linking
#pragma comment( lib, "glu32.lib" )								// Search For GLu32.lib While Linking

#ifndef CDS_FULLSCREEN											// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4										// Compilers. By Defining It This Way,
#endif															// We Can Avoid Errors

long screen_w;
long screen_h;

GL_Window*	g_window;
Keys*		g_keys;

Timer game_timer;

int num_players = 10;
Player player[10];
Backdrop background;

int num_asteroids = 500;
Asteroid asteroid[500];

int num_smoke = 800;
Smoke smoke[800];

int num_crates = 50;
Crate crate[50];

float angle= 0.0f;


float zoom = -5.0f;


bool render_done = true;

BOOL DestroyWindowGL (GL_Window* window);
BOOL CreateWindowGL (GL_Window* window);
//ENDROACH

//Thread thread_physics;

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen


//Network
NetMan net_man;


//Timing.
float ticks = 0;
float ticks_alive = 0;		//Timing for Network alive message.
float ticks_netupdate = 0;	//Timing for Network game update.
float t_target = 20000;

float time_physics;

float last_time_off;
float average_time_off;


/*
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);




VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired){
	if (render_done){
		if (g_window != NULL){
			//Update();
			//SendMessage(g_window->hWnd, WM_TIMER,0,0);	    
			//Draw();
				//Update(0);
		}else{
			//Wait some more.

		}
	}
}
*/



void TerminateApplication (GL_Window* window)							// Terminate The Application
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

void ToggleFullscreen (GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

void ReshapeGL (int width, int height)									// Reshape The Window When It's Moved Or Resized
{
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity ();													// Reset The Projection Matrix
	gluPerspective(60, (float)width/(float)height, 1,  50);
	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity ();													// Reset The Modelview Matrix

	screen_w  = width;
	screen_h  = height;
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth		= width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight		= height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

BOOL CreateWindowGL (GL_Window* window)									// This Code Creates Our OpenGL Window
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		1,																// Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};


	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	//freopen("CONOUT$", "w",stderr);

	

	/*
	hTimerQueue = CreateTimerQueue();
	int arg = 123;
	CreateTimerQueueTimer( &hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, &arg , 100, 30, 0);
	*/
	RECT windowRect = {0, 0, window->init.width, window->init.height};	// Define Our Window Coordinates

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		GetDesktopResolution(window->init.width, window->init.height);
		windowRect. left = 0;
		windowRect.top = 0;
		windowRect.right = window->init.width;
		windowRect.bottom =window->init.height;		

		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	// Create The OpenGL Window
	window->hWnd = CreateWindowEx (windowExtendedStyle,					// Extended Style
								   window->init.application->className,	// Class Name
								   window->init.title,					// Window Title
								   windowStyle,							// Window Style
								   0, 0,								// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   HWND_DESKTOP,						// Desktop Is Window's Parent
								   0,									// No Menu
								   window->init.application->hInstance, // Pass The Window Instance
								   window);

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	window->hDC = GetDC (window->hWnd);									// Grab A Device Context For This Window
	if (window->hDC == 0)												// Did We Get A Device Context?
	{
			// Failed
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

//ROACH
	/*
	Our first pass, Multisampling hasn't been created yet, so we create a window normally
	If it is supported, then we're on our second pass
	that means we want to use our pixel format for sampling
	so set PixelFormat to arbMultiSampleformat instead
  */
	if(!arbMultisampleSupported)
	{
		PixelFormat = ChoosePixelFormat (window->hDC, &pfd);				// Find A Compatible Pixel Format
		if (PixelFormat == 0)												// Did We Find A Compatible Format?
		{
			// Failed
			ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
			window->hDC = 0;												// Zero The Device Context
			DestroyWindow (window->hWnd);									// Destroy The Window
			window->hWnd = 0;												// Zero The Window Handle
			return FALSE;													// Return False
		}

	}
	else
	{
		PixelFormat = arbMultisampleFormat;
	}
//ENDROACH

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	window->hRC = wglCreateContext (window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext (window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	
//ROACH
	/*
	Now that our window is created, we want to queary what samples are available
	we call our InitMultiSample window
	if we return a valid context, we want to destroy our current window
	and create a new one using the multisample interface.
	*/
	if(!arbMultisampleSupported && CHECK_FOR_MULTISAMPLE)
	{
	
		if(InitMultisample(window->init.application->hInstance,window->hWnd,pfd))
		{
			
			DestroyWindowGL (window);
			return CreateWindowGL(window);
		}
	}

//ENDROACH

	ShowWindow (window->hWnd, SW_NORMAL);								// Make The Window Visible
	window->isVisible = TRUE;											// Set isVisible To True

	ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory (window->keys, sizeof (Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount ();							// Get Tick Count

	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL (GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{	
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent (window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext (window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context

			}
			ReleaseDC (window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings (NULL,0);									// Switch Back To Desktop Resolution
		ShowCursor (TRUE);												// Show The Cursor
	}	
	return TRUE;														// Return True
}

// Process Window Message Callbacks
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLong (hWnd, GWL_USERDATA));

	switch (uMsg)														// Evaluate Window Message
	{
		case WM_SYSCOMMAND:												// Intercept System Commands
		{
			switch (wParam)												// Check System Calls
			{
				case SC_SCREENSAVE:										// Screensaver Trying To Start?
				case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
				return 0;												// Prevent From Happening
			}
			break;														// Exit
		}
		return 0;														// Return

		case WM_CREATE:													// Window Creation
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, (LONG)(window));

		}
		return 0;
		
		case WM_PAINT:
			//We must call Begin and End Paint, otherwise this function will just get called forever.
			BeginPaint(window->hWnd,&ps);
			EndPaint(window->hWnd,&ps);		
			
		return 0;

		case WM_TIMER:
			if (render_done){
				//Draw();
				//Update();
				
				
			}
		return 0;

		case WM_CLOSE:													// Closing The Window
			TerminateApplication(window);								// Terminate The Application
		return 0;														// Return

		case WM_SIZE:													// Size Action Has Taken Place
			switch (wParam)												// Evaluate Size Action
			{
				case SIZE_MINIMIZED:									// Was Window Minimized?
					window->isVisible = FALSE;							// Set isVisible To False
				return 0;												// Return

				case SIZE_MAXIMIZED:									// Was Window Maximized?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return

				case SIZE_RESTORED:										// Was Window Restored?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = TRUE;					// Set The Selected Key (wParam) To True
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = FALSE;					// Set The Selected Key (wParam) To False
				return 0;												// Return
			}
		break;															// Break

		case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;															// Break
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass (Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= application->className;				// Sets The Applications Classname
	if (RegisterClassEx (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}



void load(){
	srand (time(NULL));
	

	NetMan::init(player,num_players);	

	//Init everything.
	Smoke::init();
	Crate::init();

	//Init asteroids with pointer to global Arrays
	Asteroid::init(&asteroid[0],num_asteroids);
	Asteroid::initSmoke(&smoke[0],num_smoke);
	Asteroid::initCrate(&crate[0],num_crates);

	//Set players
	player[1].ship.color.b = 1;
	player[1].ship.color.r = 0;
	player[1].ship.position.x = 1;

	player[0].ship.position.x = -2;
	player[1].ship.position.x = 2;

	player[0].ship.alive = true;
	player[1].ship.alive = false;

	//player[1].ship.LoadShip("bitmaps/ship2.bmp");

	background.Generate();

	vect2 rnd;
	for (int i=0;i<num_asteroids;i++){
		rnd.x = (rand()%500 - 250)/5.0f;
		rnd.y = (rand()%500 - 250)/5.0f;
		asteroid[i].SetPosition(rnd);
	}

	for (int i=num_asteroids;i>250;i--){
		asteroid[i].alive = false;
	}

	for (int i=1;i<num_crates;i++){
		crate[i].alive = false;
	}

	
}






void startup(){	
	BuildFont(g_window->hDC,g_window->hWnd);

	net_man.startup();
}

// Program Entry (WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter

	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

	// Fill Out Window
	ZeroMemory (&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys					= &keys;								// Window Key Structure
	window.init.application		= &application;							// Window Application
	window.init.title			= "OMG It's ALIVE!";	// Window Title
	window.init.width			= 720;									// Window Width
	window.init.height			= 520;									// Window Height
	window.init.bitsPerPixel	= 32;									// Bits Per Pixel
	window.init.isFullScreen	= TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory (&keys, sizeof (Keys));									// Zero keys Structure

	load();

	// Ask The User If They Want To Start In FullScreen Mode?
	//if (MessageBox (HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	//{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	//}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass (&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default
	
	//Setup thread:
	//thread_physics.bind(physics_loop);
	

	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL (&window) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (Initialize (&window, &keys) == FALSE)					// Call User Intialization
			{
				// Failure
				TerminateApplication (&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	// Initialize was a success
				
				startup();
				//thread_physics.start();
				
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage (&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage ();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{
							
							Update();
							Draw();									// Draw Our Scene
							//SwapBuffers(g_window->hDC);
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			// Application Is Finished
			Deinitialize ();											// User Defined DeInitialization

			DestroyWindowGL (&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	UnregisterClass (application.className, application.hInstance);		// UnRegister Window Class
	return 0;
}																		// End Of WinMain()



BOOL Initialize (GL_Window* window, Keys* keys)					// Any GL Init Code & User Initialiazation Goes Here
{
	g_window	= window;
	g_keys		= keys;

	// Start Of User Initialization
	angle		= 0.0f;											// Set Starting Angle To Zero

	glViewport(0 , 0,window->init.width ,window->init.height);	// Set Up A Viewport
	glMatrixMode(GL_PROJECTION);								// Select The Projection Matrix
	glLoadIdentity();											// Reset The Projection Matrix
	gluPerspective(60, (float)window->init.width/(float)window->init.height, 1,  50); // Set Our Perspective
	glMatrixMode(GL_MODELVIEW);									// Select The Modelview Matrix
	glLoadIdentity();											// Reset The Modelview Matrix

	//glEnable(GL_DEPTH_TEST);									// Enable Depth Testing

	glShadeModel(GL_SMOOTH);									// Select Smooth Shading

	glClearColor(0.0f, 0.0f, 0.0f, 0.5);						// Set The Clear Color To Black

	

	return TRUE;												// Return TRUE (Initialization Successful)
}

void Deinitialize (void)										// Any User DeInitialization Goes Here
{
}

void usleep(__int64 usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}


//Change zoom based on speed.
void DynamicZoom(float tusec){
	float zoom_target = -2.7 - vect_abs(&player[0].ship.velocity);
	if (zoom_target < -8){
		zoom_target = -8;
	}
	float diff = zoom_target - zoom;
	diff = sqrtf(abs(diff)) * (diff/40) * (tusec/40000);
	zoom += diff;	
}

void physics_loop(void){
	while(1){
		Update();
	}
}



void Update(){	
	float tusec = game_timer.getElapsedTimeInMicroSec();

	//Keep track.
	last_time_off = t_target - tusec;
	
	if (ticks > 200000){	
		average_time_off += last_time_off;
		average_time_off /= 2;	

	}
	

	if (tusec > 50000){
		tusec = 50000;
	}else if (tusec < t_target){
		usleep(t_target-tusec);
		tusec = game_timer.getElapsedTimeInMicroSec();
	}	

	
	game_timer.start();


	//printf("Msec: %i\n",milliseconds);
	//if(g_keys->keyDown [VK_SPACE] == TRUE)
		//domulti=!domulti;

	if(g_keys->keyDown ['M'] == TRUE)
		doangle=!doangle;

	if(g_keys->keyDown ['D'] == TRUE){
		player[0].Print();
	}

	if(g_keys->keyDown ['Z'] == TRUE){
		if (g_keys->keydown_time['Z'] > 0){
			g_keys->keydown_time['Z'] -=tusec;
		}else{
			dodynamiczoom = !dodynamiczoom;
			g_keys->keydown_time['Z'] = 100000;
		}
	}
	

	if(g_keys->keyDown ['P'] == TRUE){
		player[0].ship.EnableShield(true);
	}

	if(g_keys->keyDown ['O'] == TRUE){
		player[0].ship.EnableShield(false);
	}

	if(g_keys->keyDown ['T'] == TRUE){
		printf("Timer: %f usec.\n",tusec);
	}

	if(g_keys->keyDown[VK_LEFT] == TRUE){
		player[0].ship.RotateImpulse(3,tusec);	
	}
	if(g_keys->keyDown[VK_RIGHT] == TRUE){
		player[0].ship.RotateImpulse(-3,tusec);	
	}

	if(g_keys->keyDown[VK_UP] == TRUE){
		player[0].ship.Move(1,tusec);	
	}
	if(g_keys->keyDown[VK_DOWN] == TRUE){
		player[0].ship.Brake(0.5,tusec);	
	}

	if(g_keys->keyDown['A'] == TRUE){
		player[0].ship.Strafe(-0.5,tusec);	
	}

	if(g_keys->keyDown['D'] == TRUE){
		player[0].ship.Strafe(0.5,tusec);	
	}

	if(g_keys->keyDown[VK_NEXT] == TRUE){
		zoom -= 0.1f;
		if (zoom < -8)
			zoom = -8;
	}

	if(g_keys->keyDown[VK_PRIOR] == TRUE){
		zoom += 0.1f;
		if (zoom > -1.2)
			zoom = -1.2;

	}

	if(g_keys->keyDown[VK_SPACE] == TRUE){
		player[0].ship.Fire(2);
		g_keys->keyDown[VK_SPACE] = false;
		
	}

	if(g_keys->keyDown[VK_NUMPAD0] == TRUE){
		player[1].ship.Fire(2);
		g_keys->keyDown[VK_NUMPAD0] = false;
		
	}

	if(g_keys->keyDown[VK_NUMPAD4] == TRUE){
		player[1].ship.RotateDeg(2,tusec);	
	}

	if(g_keys->keyDown[VK_NUMPAD6] == TRUE){
		player[1].ship.RotateDeg(-2,tusec);	
	}

	if(g_keys->keyDown[VK_NUMPAD8] == TRUE){
		player[1].ship.Move(1,tusec);	
	}

	if(g_keys->keyDown[VK_NUMPAD5] == TRUE){
		player[1].ship.Brake(.5,tusec);	
	}


	if (g_keys->keyDown [VK_ESCAPE] == TRUE)					// Is ESC Being Pressed?
	{
		TerminateApplication (g_window);						// Terminate The Program
	}

	if (g_keys->keyDown [VK_F1] == TRUE)						// Is F1 Being Pressed?
	{
		ToggleFullscreen (g_window);							// Toggle Fullscreen Mode
	}


	for (int i=0;i<num_players;i++){
		player[i].DoPhysics(tusec);	
	}

	if (player[0].ship.position.x > 100){
		player[0].ship.position.x = -100;	
	}
	if (player[0].ship.position.x < -105){
		player[0].ship.position.x = +95;	
	}
	if (player[0].ship.position.y > 105){
		player[0].ship.position.y = -95;	
	}
	if (player[0].ship.position.y < -100){
		player[0].ship.position.y = 100;	
	}

	for(int i=0;i<num_asteroids;i++){
		asteroid[i].DoPhysics(tusec);
		if (asteroid[i].position.x > 100){
			asteroid[i].position.x = -100;	
		}
		if (asteroid[i].position.x < -105){
			asteroid[i].position.x = 95;	
		}
		if (asteroid[i].position.y > 105){
			asteroid[i].position.y = -95;	
		}
		if (asteroid[i].position.y < -100){
			asteroid[i].position.y = +100;	
		}
	}

	for(int i=0;i<num_smoke;i++){
		smoke[i].DoPhysics(tusec);
	}


	for(int i=0;i<num_crates;i++){
		crate[i].DoPhysics(tusec);
	}
	
	

	
	//Update date from other players if any.
	net_man.receive();
	

	

	//Check ship collision with asteroids.
	for (int i =0;i<num_players;i++){
		if (!player[i].ship.alive){
			continue;
		}
		for(int r=0;r<num_asteroids;r++){		
			if (CheckCollision_ShipObject(&player[i].ship,&asteroid[r])){			
				
				float tot_damage = vect_abs(&player[i].ship.momentum);				
				tot_damage += vect_abs(&asteroid[r].momentum);

				tot_damage *= 0.4;

				vect2 tdam = player[i].ship.momentum;
				player[i].ship.DoDamage(tot_damage);
			
				//player[i].ship.velocity.x = player[i].ship.velocity.x * -0.1;
				//player[i].ship.velocity.y = player[i].ship.velocity.y * -0.1;			
			
				
				tdam.x *= 0.2;
				tdam.y *= 0.2;
				asteroid[r].DoDamage(tot_damage,tdam,player[i].ship.position);
				

				vect2 t = player[i].ship.momentum;
				player[i].ship.momentum = asteroid[r].momentum;
				asteroid[r].momentum = t;

			}
		}
	}

	//Asteroid on Asteroid collisions.
	
	for(int i=0;i<(num_asteroids-1);i++){
		for(int r=i+1;r<num_asteroids;r++){
			if (CheckCollision_AstAst(&asteroid[i],&asteroid[r])){		

				//float tot_damage = vect_abs(&asteroid[i].momentum);				
				//tot_damage += vect_abs(&asteroid[r].momentum);

				//tot_damage *= 0.01;
				
				vect2 t = asteroid[i].momentum;
				asteroid[i].momentum = asteroid[r].momentum;
				asteroid[r].momentum = t;

				//asteroid[r].DoDamage(tot_damage,asteroid[i].velocity,asteroid[i].position);
				//asteroid[i].DoDamage(tot_damage,asteroid[r].velocity,asteroid[r].position);
			}
		}
	}
	
	//Check collision for ech bullet.
	for (int r=0;r<num_players;r++){
		for (int i=0;i<MAX_BULLETS;i++){
			if (player[r].ship.weapon.bullet[i].alive == false)
				continue;

			//Collision on other ships.
			for (int p=0;p<num_players;p++){
				if (p!=r){
					if (player[p].ship.alive){
						if (CheckCollision(&player[r].ship.weapon.bullet[i],&player[p].ship)){			
							player[r].ship.weapon.bullet[i].alive = false;
							player[p].ship.DoDamage(20);
						}
					}
				}
			}
		
			//Asteroids
			for(int p=0;p<num_asteroids;p++){

				if (CheckCollision_Object(&player[r].ship.weapon.bullet[i],&asteroid[p])){			
					player[r].ship.weapon.bullet[i].alive = false;
					vect2 t = player[r].ship.weapon.bullet[i].velocity;
					t.x /= 10;
					t.y /= 10;
					asteroid[p].DoDamage(10,t,player[r].ship.weapon.bullet[i].position);

				
				}
			}

			//Crates
			
			for(int p=0;p<num_crates;p++){
				if (CheckCollision_Object(&player[r].ship.weapon.bullet[i],&crate[p])){			
					player[r].ship.weapon.bullet[i].alive = false;
					vect2 t = player[r].ship.weapon.bullet[i].velocity;
					t.x /= 10;
					t.y /= 10;
					crate[p].DoDamage(10,t,player[r].ship.weapon.bullet[i].position);
				}
			}
		}
	}
	

	

	


	//background.Move(player[0].ship.position);
	
	//Do dynamic zoom:
	if (dodynamiczoom){
		DynamicZoom(tusec);
	}

	//Update time timer.
	if (ticks > 200000){	
		time_physics = game_timer.getElapsedTimeInMicroSec();
		ticks = 0;
	}
	ticks += tusec;
	ticks_alive += tusec;
	ticks_netupdate += tusec;

	if (ticks_alive > 2000000){	
		net_man.send_alive();
		ticks_alive = 0;
	}

	if (ticks_netupdate > 100000){	
		//Send data	
		net_man.send();
		ticks_netupdate = 0;
	}
}

float osd_rot = 0;

void DrawOSD(void){
	glLoadIdentity();
	glClearDepth(1.0f);
	glEnable(GL_MULTISAMPLE_ARB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//Scale
	glTranslatef(0,0,-2);
	float scale = 1/20.0f;
	glScalef(scale,scale,1);

	//Get vector to origin.
	vect2 tv = player[0].ship.position;
	osd_rot = vect_ordeg(&tv);

	glRotatef(osd_rot,0,0,1);
	glTranslatef(0,20,0);
	osd_rot += 0.1f;

	//Gray bar.
	glColor4f(1.0f,1.0f,1.0f,0.1f);	
	glBegin(GL_TRIANGLES);
		glVertex2f(-1,-1);
		glVertex2f(0,1);
		glVertex2f(1,-1);		
	glEnd();

}

void DrawSide(void){
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5);						// Set The Clear Color To Black
	//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();											// Reset The View	

	glClearDepth(1.0f);

	glEnable(GL_MULTISAMPLE_ARB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	
	
	float aspect = (float)screen_w / float(screen_h);	
	//glScalef(.5,.5,1);
	glScalef(aspect,-1,1);
	glTranslatef(-1,-1,-1.72);
	
	
	
	
	glScalef(2.0f/(float)(screen_w),2.0f/(float)(screen_h),1);

	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);

	//Gray bar.
	glColor4f(1.0f,1.0f,1.0f,0.05f);	
	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(0,screen_h);
		glVertex2f(200,screen_h);
		glVertex2f(200,0);
	glEnd();
	
	//Health:

	glColor4f(1.0f,1.0f,1.0f,0.2);	
	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(0,50);
		glVertex2f(200,50);
		glVertex2f(200,0);
	glEnd();

	glColor4f(0,1.0f,0.1f,0.3);	
	float barw = player[0].ship.health/player[0].ship.health_max;
	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(0,25);
		glVertex2f(barw*200,25);
		glVertex2f(barw*200,0);
	glEnd();

	glColor4f(0,0.3f,1.0f,0.3);	
	barw = player[0].ship.shield/player[0].ship.shield_max;
	glBegin(GL_QUADS);
		glVertex2f(0,25);
		glVertex2f(0,50);
		glVertex2f(barw*200,50);
		glVertex2f(barw*200,25);
	glEnd();
	
	glLoadIdentity();	
	g_font.setColor(1.0f, 1.0f, 1.0f);
	glPrintAA(10,2,"%7.1f/%7.1f",player[0].ship.health,player[0].ship.health_max);	
	glPrintAA(10,25,"%7.1f/%7.1f",player[0].ship.shield,player[0].ship.shield_max);	
	glPrintAA(10,50,"Screen:  %i x %i",screen_w,screen_h);	
	glPrintAA(10,70,"Render Time: %6.0f/%6.0f",average_time_off,t_target);	
	glPrintAA(10,90,"Physics Time: %6.0f",time_physics);
}



void Draw(void)												// Draw The Scene
{
	render_done = false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.5);						// Set The Clear Color To Black
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();											// Reset The View	

	glClearDepth(1.0f);

	glEnable(GL_MULTISAMPLE_ARB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	

	

	/*
	for(float i=-0;i<1;i++){
		for(float j=-0;j<1;j++)
		{
			glPushMatrix();
			glTranslatef(i*2.0f,j*2.0f,-5.0f);
			glRotatef(angle,0.f,0.f,1.f);
				glBegin(GL_QUADS);
				glColor3f(1.0f,0.0f,0.0f);glVertex3f(i,j,0.0f);
				glColor3f(0.0f,1.0f,0.0f);glVertex3f(i + 2.0f,j,0.0f);
				glColor3f(0.0f,0.0f,1.0f);glVertex3f(i + 2.0f,j + 2.0f,0.0f);
				glColor3f(1.0f,1.0f,1.0f);glVertex3f(i,j + 2.0f,0.0f);
				glEnd();
			glPopMatrix();
		}
	}*/

	//Draw Square.
	/*
	glBegin(GL_QUADS);
	glColor4f(1.0f,0.0f,0.0f,0.2f);glVertex3f(-1,1,-10.0f);
	glColor4f(0.0f,1.0f,0.0f,0.2f);glVertex3f(-1,-1,-10.0f);
	glColor4f(0.0f,0.0f,1.0f,0.2f);glVertex3f(1,-1,-10.0f);
	glColor4f(1.0f,1.0f,1.0f,0.2f);glVertex3f(1,1,-10.0f);
	glEnd();
	*/

	
	
	glPushMatrix();

	background.Render(player[0].ship.position);
	glTranslatef(0,0,zoom);
	glScalef(0.15,0.15,1);
	
	/*
	*/
	/*
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D, textureid);
	glBegin(GL_QUADS);
		 glTexCoord2f(0.0, 0.0);
		 glVertex3f(0.0, 0.0, 0.0f);
		 glTexCoord2f(1.0, 0.0);
		 glVertex3f(1.0, 0.0, 0.0f);
		 glTexCoord2f(1.0, 1.0);
		 glVertex3f(1.0, 1.0, 0.0f);
		 glTexCoord2f(0.0, 1.0);
		 glVertex3f(0.0, 1.0, 0.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);*/
	
	//Render all players relative to player[0].

	for(int i =0;i<num_players;i++){
		player[i].Render(player[0].ship.position,false);
		
	}

	Asteroid::PreRender();
	for(int i=0;i<num_asteroids;i++){
		asteroid[i].Render(player[0].ship.position);
	}

	Crate::PreRender();
	for(int i=0;i<num_crates;i++){
		crate[i].Render(player[0].ship.position);
	}
	
	//Render Smoke
	Smoke::PreRender();
	/*
	for(int i=0;i<num_asteroids;i++){
		asteroid[i].RenderSmoke(player[0].ship.position);
	}

	for(int i=0;i<num_crates;i++){
		crate[i].RenderSmoke(player[0].ship.position);
	}*/
	for(int i=0;i<num_smoke;i++){
		smoke[i].Render(player[0].ship.position);
	}


	
	glPopMatrix();
	//background.Move(player[0].ship.position);
	
	DrawSide();
	DrawOSD();
	
	if(doangle)
		angle+=0.05f;

	//glFlush ();													// Flush The GL Rendering Pipeline
	
	// ROACH
	if(domulti)
		glDisable(GL_MULTISAMPLE_ARB);
	// ENDROACH

	SwapBuffers(g_window->hDC);
	//glFinish();
	render_done = true;
}



