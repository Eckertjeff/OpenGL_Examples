
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#define GLSL(src) "#version 330 core\n" #src // Writing glsl shaders directly into character arrays in-file for examples.

#define SCREEN_HEIGHT	480
#define SCREEN_WIDTH	640
#define WINDOW_NAME		"Moving Square"

#define SQUARE_SIZE		50

struct s_polyF4 {
	unsigned long *tag;
	unsigned char r0, g0, b0;
	unsigned char code;
	GLfloat x0, y0;
	GLfloat x1, y1;
	GLfloat x2, y2;
	GLfloat x3, y3;
};

//to help translate input into a common type
enum inputTypes { UP, DOWN, LEFT, RIGHT, CHANGE_COLOR, NONE };
enum inputTypes g_decodedInput = NONE;

//prototypes
//initializes graphics
void *initGraphics();
//initialize input
void initInput(void *window);
//get translated input
enum inputTypes getInput();
//generate primitive quad
struct s_polyF4 genPrimitive();
//move the primitive passed based on input from keyboard or controller
void movePrimitive(struct s_polyF4 *f4, enum inputTypes decodedInput);
//converts psx coords to opengl array
void convertPrimQuadVertices(struct s_polyF4 *f4, GLfloat *vertexArray);
//check if the display is still live.
int displayLive(void *window);
//callback to decode input
static void inputCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);

int main( void )
{
	void *window = initGraphics();

	initInput(window);

	struct s_polyF4 polyF4;
	GLfloat vertices[12];

	polyF4 = genPrimitive();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	

	// Writing shaders in file for example, should be loaded from a separate file.
	// Create and compile the vertex shader
	const char* vertexSource = GLSL(
		in vec2 position;

		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
		}
	);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	// Create and compile the fragment shader
	const char* fragmentSource = GLSL(
		out vec4 color;

		void main() {
			color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
	);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "color");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);


	while(displayLive(window)) // Check if the window was closed
	{
		// Check to see if we need to move the primitive
		movePrimitive(&polyF4, getInput());

		// Clear the screen to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Do the vertex conversion, and send it to the gpu.
		convertPrimQuadVertices(&polyF4, vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Draw two triangles from the 6 vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Swap buffers and poll window events
		glfwSwapBuffers((GLFWwindow *)window);
		glfwPollEvents();
	}

	// Delete allocated resources
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void *initGraphics()
{
	GLFWwindow *window = NULL;
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.");
		getchar();
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	//glOrtho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, -1.0f); // Doesn't effect modern opengl in the same way.

	return window;
}

void initInput(void *window)
{
	if (window != NULL)
	{
		glfwSetKeyCallback((GLFWwindow *)window, inputCallback);
	}
}

struct s_polyF4 genPrimitive()
{
	struct s_polyF4 temp;

	#ifdef psx
	SetPolyF4((POLY_F4 *)&temp);
	#endif

	temp.r0 = 0;
	temp.g0 = 0;
	temp.b0 = 255;

	/*temp.x0 = 0;
	temp.y0 = 0;
	temp.x1 = SQUARE_SIZE;
	temp.y1 = 0;
	temp.x2 = 0;
	temp.y2 = SQUARE_SIZE;
	temp.x3 = SQUARE_SIZE;
	temp.y3 = SQUARE_SIZE;*/
	temp.x0 = -0.5f;
	temp.y0 = -0.5f;
	temp.x1 = -0.5f;
	temp.y1 = 0.5f;
	temp.x2 = 0.5f;
	temp.y2 = -0.5f;
	temp.x3 = 0.5f;
	temp.y3 = 0.5f;

	return temp;
}

enum inputTypes getInput()
{
	enum inputTypes temp = g_decodedInput;
	g_decodedInput = NONE;
	return temp;
}

void movePrimitive(struct s_polyF4 *f4, enum inputTypes decodedInput)
{
	switch (decodedInput)
	{
	case UP:
		f4->y0 = f4->y0 + .01f;
		f4->y1 = f4->y1 + .01f;
		f4->y2 = f4->y2 + .01f;
		f4->y3 = f4->y3 + .01f;
		if (f4->y0 > 1)
		{
			f4->y0 = -2.0f;
			f4->y1 = -1.0f;
			f4->y2 = -2.0f;
			f4->y3 = -1.0f;
		}
		break;
	case DOWN:
		f4->y0 = f4->y0 - .01f;
		f4->y1 = f4->y1 - .01f;
		f4->y2 = f4->y2 - .01f;
		f4->y3 = f4->y3 - .01f;
		if (f4->y1 < -1)
		{
			f4->y0 = 1.0f;
			f4->y1 = 2.0f;
			f4->y2 = 1.0f;
			f4->y3 = 2.0f;
		}
		break;
	case LEFT:
		f4->x0 = f4->x0 - .01f;
		f4->x1 = f4->x1 - .01f;
		f4->x2 = f4->x2 - .01f;
		f4->x3 = f4->x3 - .01f;
		if (f4->x2 < -1)
		{
			f4->x0 = 1.0f;
			f4->x1 = 1.0f;
			f4->x2 = 2.0f;
			f4->x3 = 2.0f;
		}
		break;
	case RIGHT:
		f4->x0 = f4->x0 + .01f;
		f4->x1 = f4->x1 + .01f;
		f4->x2 = f4->x2 + .01f;
		f4->x3 = f4->x3 + .01f;
		if (f4->x0 > 1)
		{
			f4->x0 = -2.0f;
			f4->x1 = -2.0f;
			f4->x2 = -1.0f;
			f4->x3 = -1.0f;
		}
		break;
	case CHANGE_COLOR:
		//check if it has been greater than 1 second since glfw time has been reset
		if (glfwGetTime() > 1.0f)
		{
			glfwSetTime(0.0f);
			f4->r0 = rand() % 256;
			f4->g0 = rand() % 256;
			f4->b0 = rand() % 256;
		}
		break;
	default:
		break;
		}
	}

void display(void *window, struct s_polyF4 *f4)
{
	//WIP
}



void convertPrimQuadVertices(struct s_polyF4 *f4, GLfloat *vertexArray)
{
	// We need to convert from the psx struct to an array opengl can use.
	vertexArray[0] = f4->x0;
	vertexArray[1] = f4->y0;
	vertexArray[2] = f4->x1;
	vertexArray[3] = f4->y1;
	vertexArray[4] = f4->x2;
	vertexArray[5] = f4->y2;
	vertexArray[6] = f4->x2;
	vertexArray[7] = f4->y2;
	vertexArray[8] = f4->x3;
	vertexArray[9] = f4->y3;
	vertexArray[10] = f4->x1;
	vertexArray[11] = f4->y1;
}

int displayLive(void *window)
{
	return (glfwWindowShouldClose((GLFWwindow *)window) == 0 ? 1 : 0);
}

//keyboard input callback to decode key pressed into are decoded type.
static void inputCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
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