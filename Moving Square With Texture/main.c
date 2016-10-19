
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//size of the square on screen, will always be 50 pixels of the total screen size
#define SQUARE_SIZE	200
//screen defines based on platform
#define SCREEN_HEIGHT	480
#define SCREEN_WIDTH	640
#define WINDOW_NAME	"MOVING SQUARE"

//define polyF4 for openGL, this is the struct the psx uses
struct s_polyF4 {
  unsigned long *tag;
  unsigned char r0, g0, b0;
  unsigned char code;
  short x0,y0;
  short x1,y1;
  short x2,y2;
  short x3,y3;
};

//to help translate input into a common type
enum inputTypes {UP, DOWN, LEFT, RIGHT, CHANGE_COLOR, NONE};
enum inputTypes g_decodedInput = NONE;

//prototypes
//initializes graphics
void *initGraphics();
//initialize input
void initInput(void *window);
//get translated input
enum inputTypes getInput();
//move the primitive passed based on input from keyboard or controller
void movePrimitive(struct s_polyF4 *f4, enum inputTypes decodedInput);
//generate primite, a square in this case
struct s_polyF4 genPrimitive();
//update display window with primitive
void display(void *window, struct s_polyF4 *f4);
//check if the display is still live (psx is infinite)
int displayLive(void *window);
//cleanup for glfw, does nothing for psx
void cleanup(void *window);
//define a DramPrim for linux using openGL (already defined for psx)
void DrawPrim(struct s_polyF4 *f4);
//callback to decode input
static void inputCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);

//application, takes no arguments at this point
int main(int argc, char *argv[])
{
  struct s_polyF4 polyF4;

  void *window = initGraphics();

  initInput(window);

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  polyF4 = genPrimitive();
  GLfloat g_vertex_buffer_data[] = 
  {
  	0.5f, 1.0f, 0.0f,
  	1.0f, 0.0f, 0.0f,
  	0.0f, 0.0f, 0.0f,
  };
  // GLfloat g_vertex_buffer_data[] = 
  // {
  // 	-1.0f, 1.0f, 0.0f,
  // 	1.0f, -1.0f, 0.0f,
  // 	0.0f, 1.0f, 0.0f,
  // };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  while(displayLive(window))
  {
    //movePrimitive(&polyF4, getInput());
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
    	0,
    	3,
    	GL_FLOAT,
    	GL_FALSE,
    	0,
    	(void*)0
    	);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  cleanup(window);

  return 0;
}

//initialize graphics for either psx or linuxF
void *initGraphics()
{
//glfw init
int returnValue = 0;
GLenum glewReturnValue = 0;
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
GLFWwindow *window = NULL;

returnValue = glfwInit();

if(returnValue == GL_FALSE)
{
  fprintf(stderr, "Failed to initialize GLFW.\n");
  exit(EXIT_FAILURE);
}

window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME, NULL, NULL);

if(window == NULL)
{
  fprintf(stderr, "Failed to create window\n");
  glfwTerminate();
  exit(EXIT_FAILURE);
}

glfwMakeContextCurrent(window);
glewExperimental=GL_TRUE;
//vsync?
glfwSwapInterval(1);

glewReturnValue = glewInit();

if(glewReturnValue != GLEW_OK)
{
  fprintf(stderr, "Failed to initialize GLEW... %s\n", glewGetErrorString(glewReturnValue));
  glfwTerminate();
  exit(EXIT_FAILURE);
}

//setup area using openGL calls
//set background color
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
glLoadIdentity();
//change drawing area to one that matches the playstation
//glOrtho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, -1.0f);
//used to time the color change button press (set to allow initial press to go regardless of time)
glfwSetTime(1.0f);

return window;
}

//setup input based on arch
void initInput(void *window)
{
    //glfw
    if(window != NULL)
    {
      glfwSetKeyCallback((GLFWwindow *)window, inputCallback);
    }
}

//get the input and reset it to NONE
enum inputTypes getInput()
{
  enum inputTypes temp = g_decodedInput;
  g_decodedInput = NONE;
  return temp;
}

//universal method of moving the primitive on screen (at this point does a warp to the other side if you touch it)
void movePrimitive(struct s_polyF4 *f4, enum inputTypes decodedInput)
{
	switch(decodedInput)
	{
		case UP:
		f4->y0--;
		f4->y1--;
		f4->y2--;
		f4->y3--;
		if(f4->y0 < 0)
		{
			f4->y0 = SCREEN_HEIGHT - SQUARE_SIZE;
			f4->y1 = SCREEN_HEIGHT - SQUARE_SIZE;
			f4->y2 = SCREEN_HEIGHT;
			f4->y3 = SCREEN_HEIGHT;
		}
		break;
		case DOWN:
		f4->y0++;
		f4->y1++;
		f4->y2++;
		f4->y3++;
		if(f4->y2 > SCREEN_HEIGHT)
		{
			f4->y0 = 0;
			f4->y1 = 0;
			f4->y2 = SQUARE_SIZE;
			f4->y3 = SQUARE_SIZE;
		}
		break;
		case LEFT:
		f4->x0--;
		f4->x1--;
		f4->x2--;
		f4->x3--;
		if(f4->x0 < 0)
		{
			f4->x0 = SCREEN_WIDTH - SQUARE_SIZE;
			f4->x1 = SCREEN_WIDTH;
			f4->x2 = SCREEN_WIDTH - SQUARE_SIZE;
			f4->x3 = SCREEN_WIDTH;
		}
		break;
		case RIGHT:
		f4->x0++;
		f4->x1++;
		f4->x2++;
		f4->x3++;
		if(f4->x1 > SCREEN_WIDTH)
		{
			f4->x0 = 0;
			f4->x1 = SQUARE_SIZE;
			f4->x2 = 0;
			f4->x3 = SQUARE_SIZE;
		}
		break;
		case CHANGE_COLOR:
  		//check if it has been greater than 1 second since glfw time has been reset
		if(glfwGetTime() > 1.0f)
		{
			glfwSetTime(0.0f);
			f4->r0 = rand()%256;
			f4->g0 = rand()%256;
			f4->b0 = rand()%256;
		}
		break;
		default:
		break;
	}
}

//generate primitive
struct s_polyF4 genPrimitive()
{
  struct s_polyF4 temp;

  temp.r0 = 0;
  temp.g0 = 0;
  temp.b0 = 255;

  temp.x0 = 0;
  temp.y0 = 0;
  temp.x1 = SQUARE_SIZE;
  temp.y1 = 0;
  temp.x2 = 0;
  temp.y2 = SQUARE_SIZE;
  temp.x3 = SQUARE_SIZE;
  temp.y3 = SQUARE_SIZE;

  return temp;

}

//display primitive, window is only used for glfw, pass NULL for psx
void display(void *window, struct s_polyF4 *f4)
{
    //GLFW
    //draw primitive function based on openGL calls
    DrawPrim(f4);
    if(window != NULL)
    {
      //swap draw and display buffer (back and front buffer)
      glfwSwapBuffers((GLFWwindow *)window);
      //poll events in the winow system
      glfwPollEvents();
    }
}

//only needed for glfw, does nothing for the psx
void cleanup(void *window)
{
    if(window != NULL)
    {
      glfwDestroyWindow((GLFWwindow *)window);
    }
}

//check if the window is still open, since the PSX does not have a window, it returns a 1 to run infinitely
int displayLive(void *window)
{
    return (glfwWindowShouldClose((GLFWwindow *)window) == 0 ? 1 : 0);
}
//openGL DrawPrim function, mimics what the PSX version does (theirs is native aka, build in)
void DrawPrim(struct s_polyF4 *f4)
{
	// float verticies[] = {
	// 	f4->x0, f4->y0,
	// 	f4->x1, f4->y1,
	// 	f4->x3, f4->y3
	// };
	// GLuint vbo;
	// glGenBuffers(1, &vbo);
	// glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
	// glDrawArrays(GL_TRIANGLES, 0, 3);
}

//keyboard input callback to decode key pressed into are decoded type.
static void inputCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
	  switch(key)
	  {
	case GLFW_KEY_UP:
	  g_decodedInput = UP;
	  break;
	case GLFW_KEY_DOWN:
	  g_decodedInput = DOWN;
	  break;
	case GLFW_KEY_LEFT:
	  g_decodedInput = LEFT;
	  break;
	case GLFW_KEY_RIGHT:
	  g_decodedInput = RIGHT;
	  break;
	case GLFW_KEY_X:
	  g_decodedInput = CHANGE_COLOR;
	  break;
	default:
	  g_decodedInput = NONE;
	  break;
	  }
	}
}