
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
// Include SOIL
#include <SOIL.h>

#define GLSL(src) "#version 330 core\n" #src // writing glsl shaders directly into character arrays in-file for examples.

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
enum inputTypes { UP, DOWN, LEFT, RIGHT, UPLEFT, UPRIGHT, DOWNLEFT, DOWNRIGHT, CHANGE_COLOR, NONE };
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

int main(void)
{
	void *window = initGraphics();
	initInput(window);

	/*struct s_polyF4 polyF4;
	polyF4 = genPrimitive();*/
	//GLfloat vertices[12];
	GLfloat vertices[] = {
		// Positions          // Colors R,G,B,   // Texture Coords
		0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // Top Right
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right
		-0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,   // Bottom Left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f    // Top Left 
	};

	// Using indicies and the element buffer allows us to reuse verticies so we can draw a quad with four verticies declared instead of the six required for two distinct triangles.
	GLuint indices[] = {  // Note that we start from 0.
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	}; // Using indices and 
	   // ***Begin Setup of Shaders***
	   // Writing shaders in file for example, should be loaded from a separate file.
	   // Create vertex shader
	const char* vertexSource = GLSL(
		layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 color;
	layout(location = 2) in vec2 texCoord;

	out vec3 ourColor;
	out vec2 TexCoord;

	void main()
	{
		gl_Position = vec4(position, 1.0f);
		ourColor = color;
		// We swap the y-axis by substracing our coordinates from 1. This is done because most images have the top y-axis inversed with OpenGL's top y-axis.
		// TexCoord = texCoord;
		TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
	}
	);
	// Create fragment shader
	const char* fragmentSource = GLSL(
		in vec3 ourColor;
	in vec2 TexCoord;

	out vec4 color;

	// Texture samplers
	uniform sampler2D ourTexture1;

	void main()
	{
		// ourColor that we declared in the verticies could be, but is not used in this shader. Instead we're just using the texture.
		color = texture(ourTexture1, TexCoord);
	}
	);


	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	// Fragment Shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	// ***End Setup of Shaders***

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an Element Buffer Object, and copy the indicies data into it
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// Now we need to declare where our attributes are located in our vertex array object
	// glVertexAttribPointer(index, size, type, normalized, stride, offset)
	// index is the index to the generic vertex attribute to be modified. Used in glEnableVertexAttribArray(index);

	// Position is three floats starting at 0 [x,y,z].
	// The stride or the distance to next position is 32bytes or 8*sizeof(GLfloat).
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Color is three floats, starting at an offset of 3 floats [R,G,B]
	// The stride is 32bytes to the next color set
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// TexCoords are two floats, starting at an offset of 6 floats [x,y]
	// The stride is 32 bytes to the next texture cord
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

						  // ***Begin Texture***
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	int width, height;
	unsigned char* image = SOIL_load_image("batman.png", &width, &height, 0, SOIL_LOAD_RGB);
	// Should Error Check for non-loaded texture.

	// Send texture information to our texture buffer and generate the mipmap.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image); // We don't need the image loaded as a char array anymore, so we can free it.
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
									 // **End Texture***

	while (displayLive(window)) // Check if the window was closed
	{
		// Check to see if we need to move the primitive
		//movePrimitive(&polyF4, getInput());

		// Clear the screen to greenish, our texture is black.
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Do the vertex conversion, and send it to the gpu.
		//convertPrimQuadVertices(&polyF4, vertices);

		// Bind the texture and our vertex array
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(vao);

		// Draw using the elements we set up.
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // Unbind our vertex array object.

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