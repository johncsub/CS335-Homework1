//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
//indent fix
//  out of insert:
//gg=G
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <unistd.h>
#include <string>

extern "C" {
#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 400000
#define GRAVITY 0.1
#define MAX_BOXES 5
#define NEW_PARTICLES 25

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

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

struct Game {
    bool bubbler;
    bool nyan_colors;
    Particle *particle;
    int lastMouse[2];
    Shape box[MAX_BOXES];
    Shape circle;
    int n;

    Game() {
	bubbler=false;
	nyan_colors=false;
	particle = new Particle[MAX_PARTICLES];
	n=0;
	//declare a box shape
	for(int i=0;i<5;i++) {
	    box[i].width = 100;
	    box[i].height = 10;
	    box[i].center.x = 120 + i*65;
	    box[i].center.y = 500 - i*60;
	}
	circle.radius=150.0;
	circle.center.x=600;
	circle.center.y=0;

    }
    ~Game() {
	delete [] particle;
    }
};

const std::string waterfall[] = {"Requirements",
    "Design",
    "Coding",
    "Testing",
    "Maintenance"};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();

    //declare game object
    Game game;

    //start animation
    while(!done) {
	while(XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
	usleep(100);
    }
    cleanupXWindows();
    cleanup_fonts();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tCannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
	std::cout << "\n\tNo appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask |
	PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);

    //initialize fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

#define rnd() (float)rand()/(float)RAND_MAX
void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
	return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5f;
    p->velocity.x = rnd() - 0.5f;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    for(int i=0;i<NEW_PARTICLES;i++)
		makeParticle(game, e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	if (++n < 10)
	    return;
	int y = WINDOW_HEIGHT - e->xbutton.y;
	for(int i=0;i<NEW_PARTICLES;i++)
	    makeParticle(game, e->xbutton.x, y);

	game->lastMouse[0]=savex;
	game->lastMouse[1]=WINDOW_HEIGHT - savey;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.
	if(key == XK_b) {
	    game->bubbler=!(game->bubbler);
	}
	if(key == XK_h) {
	    game->nyan_colors=!(game->nyan_colors);
	}

    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if(game->bubbler) {
	for(int i=0;i<NEW_PARTICLES;i++){
	    makeParticle(game, game->lastMouse[0], game->lastMouse[1]);
	}
    }

    if (game->n <= 0)
	return;

    const float min_X = 0.65f;
    const float vel_Y = 0.425;
    for(int i=0; i<game->n;i++) {
	p = &game->particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	p->velocity.y -= GRAVITY;


	//check for collision with shapes...
	//Shape *s;
	for(int j=0; j<MAX_BOXES;j++) {
	    if(p->s.center.x>=game->box[j].center.x-game->box[j].width &&
		    p->s.center.x<=game->box[j].center.x+game->box[j].width &&
		    p->s.center.y<game->box[j].center.y+game->box[j].height &&
		    p->s.center.y>game->box[j].center.y-game->box[j].height) {

		//collision w/ box
		p->s.center.y = game->box[j].center.y+10;
		p->velocity.y *= -vel_Y;
		if(p->velocity.x < min_X) {
		    if(p->velocity.x < 0.0f)
			p->velocity.x *= -1.0f;
		    else
			p->velocity.x = min_X + ((rnd()*2.5)*0.4);
		}
	    }
	}
	// set circle collision
	float d0,d1, dist;
	d0=p->s.center.x-game->circle.center.x;
	d1=p->s.center.y-game->circle.center.y;
	dist=sqrt((d0*d0)+(d1*d1));
	if(dist <= game->circle.radius) {
	    //collision to circle!
	    //move out of collision state
	    p->s.center.x=game->circle.center.x+(d0/dist)*game->circle.radius*1.01;
	    p->s.center.y=game->circle.center.y+(d1/dist)*game->circle.radius*1.01;
	    //apply penalty
	    p->velocity.x += (d0/dist)*2.25;
	    p->velocity.y += (d1/dist)*2.25;
	}



	//check for off-screen
	if (p->s.center.y < 0.0) {
	    memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
	    //std::cout << "Off screen" << std::endl;
	    game->n--;
	}
    }
}
void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...


    const int n=40;
    static Vec vert[n]; 
    static bool first=true;
    if(first) {
	float angle=0.0, inc=(3.14159 * 2)/(float)n;
	for(int i=0;i<n;i++) {
	    vert[i].x=cos(angle)*game->circle.radius;
	    vert[i].y=sin(angle)*game->circle.radius;
	    angle+=inc;
	}
	first=false;
    }

    glBegin(GL_LINE_LOOP);
    glColor3ub(60,140,90);
    for(int i=0;i<n;i++) {
	glVertex2i(game->circle.center.x+vert[i].x,
		game->circle.center.y+vert[i].y);
    }
    glEnd();
    //draw box
    Shape *s;
    for(int i=0; i<MAX_BOXES;i++) {
	s = &game->box[i];
    glColor3ub(90,140,90);
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
    }

    float nyan_R, nyan_G, nyan_B;
    //draw all particles here
    glPushMatrix();
    if(!game->nyan_colors)
	glColor3ub(150,160,220);
    else {
	nyan_R = rnd()*255;
	nyan_G = (rnd()*255) - nyan_R;
	nyan_B = (rnd()*255) - (nyan_R + nyan_G);
	glColor3ub(nyan_R,nyan_G,nyan_B);
    }
    for(int i=0;i<game->n;i++) {
	//if(game->nyan_colors)
	//glColor3ub(rnd()*255,rnd()*255,rnd()*255);
	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }

    // draw text onto screen
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    Rect wm;
    wm.bot=WINDOW_HEIGHT-30;
    wm.left=0;
    wm.center=0;
    ggprint16(&wm, 36, 0x00ffffff,"Waterfall model");

    for(int i=0; i<MAX_BOXES;i++) {
    Shape *s;
    s = &game->box[i];
    w = s->width;
    h = s->height;
    Rect r;
    r.bot=s->center.y-h;
    r.left=s->center.x-(w/2);
    r.center=0;
    ggprint16(&r, 36, 0x00d1a319, waterfall[i].c_str());
    }

}



