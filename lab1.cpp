//Modified by: Nagi Obeid
//Date: 9-20-19

//3350 Spring 2019 Lab-1 // Homework-1
//This program demonstrates the use of OpenGL and XWindows

//Assignment is to modify this program.
//You will follow along with your instructor.

//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git

//elements we will add to program...
// .Game constructor
// .multiple particles
// .gravity
// .collision detection
// .more objects

#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"
const int MAX_PARTICLES = 2000;
const float GRAVITY    	= .05;//0.10;

//some structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

class Global {
	public:
		int xres, yres;
		Shape box; 
		//Shape box[5];
		Particle particle[MAX_PARTICLES];
		int n;
		Global();
} g;

class X11_wrapper {
	private:
		Display *dpy;
		Window win;
		GLXContext glc;
	public:
		~X11_wrapper();
		X11_wrapper();
		void set_title();
		bool getXPending();
		XEvent getXNextEvent();
		void swapBuffers();
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	srand((unsigned)time(NULL));
	init_opengl();
	//Main animation loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			check_mouse(&e);
			done = check_keys(&e);
		}
		movement();
		render();
		x11.swapBuffers();
	}
	cleanup_fonts(); //ADDED 9-8-19
	return 0;
}

//-----------------------------------------------------------------------------
//Global class functions
//-----------------------------------------------------------------------------
Global::Global()
{
	xres = 500; //800
	yres = 360; //600
	box.width = 70;
	box.height = 10;
	box.center.x = 110; //+ 5*65;
	box.center.y = 300; //- 5*60;
}

//-----------------------------------------------------------------------------
//X11_wrapper class functions
//-----------------------------------------------------------------------------
X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab1");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}
//---------------------------------------------------------------------------

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_TEXTURE_2D);	//ADDED 9-8-19
	initialize_fonts();		//ADDED 9-8-19
}

void makeParticle(int x, int y)
{
	//Add a particle to the particle system.
	//
	if (g.n >= MAX_PARTICLES)
		return;
	cout << "makeParticle() " << x << " " << y << endl;
	//set position of particle
	Particle *p = &g.particle[g.n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = ((double)rand() /(double)RAND_MAX) - 2.5;//- 0.25;//0.525;
	p->velocity.x = ((double)rand() /(double)RAND_MAX) + 0.10; //0.525;
	++g.n;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
			e->type != ButtonPress &&
			e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed.
			int y = g.yres - e->xbutton.y;
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			makeParticle(e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.
			int y = g.yres - e->xbutton.y;
			for (int i = 0 ;i < 10; i++) {
				makeParticle(e->xbutton.x, y);
			}
		}
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_a:
				//Key A was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void movement()
{

	if (g.n <= 0)
		return;

	for (int i = 0; i < g.n; i++)
	{
		Particle *p = &g.particle[i];
		p = &g.particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

		Shape *s[5];
		
		for (int j = 0; j < 5 ; j++){
		int incValue;

		switch ( j ) {
			case 0:
				incValue = 0;
				break;
			case 1:
				incValue = 40;
				break;
			case 2:
				incValue = 80;
				break;
			case 3:
				incValue = 120;
				break;
			case 4:
				incValue = 160;
				break;}
		s[j] = &g.box;
		if (p->s.center.y < s[j]->center.y - incValue + s[j]->height
				&& p->s.center.y > s[j]->center.y - incValue - s[j]->height
				&& p->s.center.x > s[j]->center.x + incValue - s[j]->width 
				&& p->s.center.x < s[j]->center.x + incValue + s[j]->width)
			p->velocity.y = -p->velocity.y / 6;
			
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			cout << "off screen" << endl;
			//g.n = 0;
			g.particle[i] = g.particle[g.n-1];
			--g.n;
		}
	}
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ub(90,140,90);
	Shape *s[5];
	float w, h;
	int incValue;
	Rect r;
	r.center = 0;
	
	for (int i = 0; i < 5 ; i++) {	
		switch ( i ) {
			case 0:
				incValue = 0;
				break;
			case 1:
				incValue = 40;
				break;
			case 2:
				incValue = 80;
				break;
			case 3:
				incValue = 120;
				break;
			case 4:
				incValue = 160;
				break;
		}
		
		s[i] = &g.box;	
		glPushMatrix();
		glTranslatef(s[i]->center.x + incValue, s[i]->center.y - incValue, s[i]->center.z);	
		w = s[i]->width;
		h = s[i]->height;
		glBegin(GL_QUADS);
		glVertex2i(-w, -h);
		glVertex2i(-w,  h);
		glVertex2i( w,  h);
		glVertex2i( w, -h);
		glEnd();
		glPopMatrix();
}

	for (int k = 0; k < g.n; k++) {
		glPushMatrix();
	//	glColor3ub(150,160,220);
		Vec *c = &g.particle[k].s.center;
		w = h = 2;
		glBegin(GL_QUADS);
		glColor3ub(150,160,220);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
	
	for (int j = 0; j < 5 ; j++) {
		switch ( j ) {
			case 0:
				incValue = 0;
				r.left = s[j]->center.x + incValue - 40;
				r.bot = s[j]->center.y - incValue - 5;
				ggprint8b(&r, 16, 0x00ff0000, "Requirements");
				break;
			case 1:
				incValue = 40;
				r.left = s[j]->center.x + incValue - 25;
				r.bot = s[j]->center.y - incValue - 5;
				ggprint8b(&r, 16, 0x00ff0000, "Design");
				break;
			case 2:
				incValue = 80;
				r.left = s[j]->center.x + incValue - 25;
				r.bot = s[j]->center.y - incValue - 5;
				ggprint8b(&r, 16, 0x00ff0000, "Coding");
				break;
			case 3:
				incValue = 120;
				r.left = s[j]->center.x + incValue - 25;
				r.bot = s[j]->center.y - incValue - 5;
				ggprint8b(&r, 16, 0x00ff0000, "Testing");
				break;
			case 4:
				incValue = 160;
				r.left = s[j]->center.x + incValue - 25;
				r.bot = s[j]->center.y - incValue - 5;
				ggprint8b(&r, 16, 0x00ff0000, "Maintenace");
				break;
		}
	}
}

