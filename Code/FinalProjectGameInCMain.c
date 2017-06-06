#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW\glfw3.h>

//PROJECT HEADERS
#include "MagicalShaders.h"


//Struct 
typedef struct Moves TMoves;
struct Moves {
	// 1 - left
	// 2 - right
	// 3 - up
	// 4 - down
	int lastPosition;
	struct Moves* lastMove;
};

//GLOBALS 
const GLint WIDTH = 900, HEIGHT = 900; //Alter this if this is too small
TMoves* movesList;
int** board;
int currentCol;
int currentRow;
int gameboard;
int leftrightpress = 0;
int updownpress = 0;
int bombleftrightpress = 0;
int bombupdownpress = 0;
float offset;
int moves = 0;
int lives = 3;



//Function Prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void createBoardArray(int);
GLfloat* createboardVertArray(int);
GLuint* createboardIndices(int);
GLfloat* createendSpaceArray(int);
GLfloat* createplayerSpaceArray(int);
GLfloat* createBombArray(int);
void displayStats(void);
void push(TMoves**, int);
int pop(TMoves**);
void printMoves(TMoves*);
void printBoard(int**, int);
void moveLeft();
void moveRight();
void moveUp();
void moveDown();
void checkBombs(int, int);


int main() {

	
	int arraysize;
	int totalIndices;
	int tmp = lives;

	GLfloat* verts;
	GLuint* indices;
	GLfloat* endSpace;
	GLfloat* playerSpace;
	GLfloat* bomb;


	do {
		printf("\nEnter the size of the board: ");
		scanf("%d", &gameboard);
	} while (gameboard < 2);

	if (gameboard % 2 == 0) {
		arraysize = ((gameboard * gameboard) / 2) * 8;
		totalIndices = ((gameboard * gameboard) / 2) * 6;
		offset = 2.0 / gameboard;
	}
	else {
		arraysize = (((gameboard * gameboard) + 1) / 2) * 8;
		totalIndices = (((gameboard * gameboard) + 1) / 2) * 6;
	}

	offset = 2.0 / gameboard;

	createBoardArray(gameboard);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Working with Modern OpenGL is a pain!", 0, 0);

	int screenWitdth, screenHeight;
	glfwGetFramebufferSize(window, &screenWitdth, &screenHeight);

	if (0 == window) {
		printf("Failed Window Creation");
		glfwTerminate();

		return 0;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {

		printf("Failed to initialise GLEW");
		return 0;
	}

	glViewport(0, 0, screenWitdth, screenHeight);

	// Build and compile our Vextex and Fragment Shader for the Gameboard
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Link shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Build and compile Vertex and Shader Program for Rainbow Shader
	GLuint RBvertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(RBvertexShader, 1, &RBvertexShaderSource, NULL);
	glCompileShader(RBvertexShader);

	GLuint RBfragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(RBfragmentShader, 1, &RBfragmentShaderSource, NULL);
	glCompileShader(RBfragmentShader);

	GLuint RBshaderProgram = glCreateProgram();
	glAttachShader(RBshaderProgram, RBvertexShader);
	glAttachShader(RBshaderProgram, RBfragmentShader);
	glLinkProgram(RBshaderProgram);

	glDeleteShader(RBvertexShader);
	glDeleteShader(RBfragmentShader);

	//Build and Compile Vertext and Shader Program for Player 
	GLuint PlayerVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PlayerVertexShader, 1, &PlayerVertexShaderSource, NULL);
	glCompileShader(PlayerVertexShader);

	GLuint PlayerFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PlayerFragmentShader, 1, &PlayerFragmentShaderSource, NULL);
	glCompileShader(PlayerFragmentShader);

	GLuint PlayerShaderProgram = glCreateProgram();
	glAttachShader(PlayerShaderProgram, PlayerVertexShader);
	glAttachShader(PlayerShaderProgram, PlayerFragmentShader);
	glLinkProgram(PlayerShaderProgram);

	glDeleteShader(PlayerVertexShader);
	glDeleteShader(PlayerFragmentShader);

	//Build and Compile Vertext and Shader Program for Bomb 
	GLuint BombVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(BombVertexShader, 1, &BombVertexShaderSource, NULL);
	glCompileShader(BombVertexShader);

	GLuint BombFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(BombFragmentShader, 1, &BombFragmentShaderSource, NULL);
	glCompileShader(BombFragmentShader);

	GLuint BombShaderProgram = glCreateProgram();
	glAttachShader(BombShaderProgram, BombVertexShader);
	glAttachShader(BombShaderProgram, BombFragmentShader);
	glLinkProgram(BombShaderProgram);

	glDeleteShader(BombVertexShader);
	glDeleteShader(BombFragmentShader);

	// Verts Array Creation
	verts = createboardVertArray(gameboard);
	indices = createboardIndices(gameboard);
	endSpace = createendSpaceArray(gameboard);
	playerSpace = createplayerSpaceArray(gameboard);
	bomb = createBombArray(gameboard);


	GLuint VBO, VAO, EBO, endVBO, endVAO, playerVBO, playerVAO, bombVBO, bombVAO;

	//Gameboard
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, arraysize * sizeof(verts), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndices * sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//EndSpace
	glGenVertexArrays(1, &endVAO);
	glGenBuffers(1, &endVBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(endVAO);
	glBindBuffer(GL_ARRAY_BUFFER, endVBO);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(endSpace), endSpace, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO

	//PlayerSpace
	glGenVertexArrays(1, &playerVAO);
	glGenBuffers(1, &playerVBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(playerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(playerSpace), playerSpace, GL_DYNAMIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO

	//BombSpace
	glGenVertexArrays(1, &bombVAO);
	glGenBuffers(1, &bombVBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s)
	glBindVertexArray(bombVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bombVBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(bomb), bomb, GL_DYNAMIC_DRAW);
	//Position attribute but no color
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);



	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		
		//DRAW OPENGL STUFF
		//Drawing gameboard
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, totalIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//Draw Endpoint
		glUseProgram(RBshaderProgram);
		glBindVertexArray(endVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		//Draw Player
		glUseProgram(PlayerShaderProgram);
		glBindVertexArray(playerVAO);
		glUniform1f(glGetUniformLocation(PlayerShaderProgram, "Xoffset"), offset * leftrightpress);
		glUniform1f(glGetUniformLocation(PlayerShaderProgram, "Yoffset"), offset * updownpress);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		


		checkBombs(leftrightpress, updownpress);
		if ( tmp > lives ) {
			bombleftrightpress = leftrightpress;
			bombupdownpress = updownpress;
			tmp = lives;
		}
		
		glUseProgram(BombShaderProgram);
		GLfloat timeValue = glfwGetTime();
		GLfloat redValue = (sin(timeValue) / 2) + 0.50;
		GLint vertexColorLocation = glGetUniformLocation(BombShaderProgram, "ourColor");
		glUniform4f(vertexColorLocation, redValue, 0.0f, 0.0f, 1.0f);
		glBindVertexArray(bombVAO);
		glUniform1f(glGetUniformLocation(BombShaderProgram, "Xoffset"), offset * bombleftrightpress);
		glUniform1f(glGetUniformLocation(BombShaderProgram, "Yoffset"), offset * bombupdownpress);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		// Swap Screen Buffers
		glfwSwapBuffers(window);

		//checkBombs(leftrightpress, updownpress);
		if (lives == 0) {
			printf("\nGAME OVER!\n");
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (*(*(board + gameboard - 1) + gameboard - 1) == 1) {
			printf("\n\n Congratulations! You have won! Your score is %d moves.\n", moves);
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	//Deallocate
	for (int i = 0; i < gameboard; i++) {
		free(*(board + i));
	}
	free(board);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &endVAO);
	glDeleteBuffers(1, &endVBO);
	glDeleteVertexArrays(1, &playerVAO);
	glDeleteBuffers(1, &playerVBO);
	glDeleteVertexArrays(1, &bombVAO);
	glDeleteBuffers(1, &bombVBO);
	glDeleteProgram(shaderProgram);
	glDeleteProgram(RBshaderProgram);
	glDeleteProgram(PlayerShaderProgram);
	glDeleteProgram(BombShaderProgram);
	free(verts);
	free(indices);
	free(endSpace);
	free(playerSpace);
	free(bomb);
	glfwTerminate();


	return 1;
}



//Function Definitions 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		moveLeft();
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		moveRight();
	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		moveUp();
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		moveDown();
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		int lastMove = pop(&movesList);

		switch (lastMove) {
		case 1://left
			moveRight();
			pop(&movesList);

			break;

		case 2://right
			moveLeft();
			pop(&movesList);

			break;

		case 3://up
			moveDown();
			pop(&movesList);

			break;

		case 4://down
			moveUp();
			pop(&movesList);

			break;

		default:
			printf("\nno more moves to go back through\n");

			break;
		}
	}
}

void createBoardArray(int boardSize) {
	int i;
	movesList = (TMoves*)malloc(sizeof(TMoves));
	movesList->lastPosition = -1;
	movesList->lastMove = 0;

	board = (int**)malloc(boardSize * sizeof(int*));
	for (i = 0; i < boardSize; i++) {
		*(board + i) = (int*)malloc(boardSize * sizeof(int));
		for (int j = 0; j < boardSize; j++) {
			*(*(board + i) + j) = 0;
		}
	}
	currentCol = 0;
	currentRow = 0;

	srand(time(NULL));
	for (i = 0; i < boardSize + 1; i++) {
		*(*(board + (rand() % boardSize)) + (rand() % boardSize)) = 2;
	}
	**board = 1;
	*(*(board + boardSize - 1) + boardSize - 1) = 0;
}


GLfloat* createplayerSpaceArray(int gameboard) {
	//GLfloat offset;
	GLfloat* playerSpaceArray;

	playerSpaceArray = (GLfloat*)calloc(18, sizeof(GLfloat));
	//offset = 2.0 / gameboard;

	playerSpaceArray[0] = -1.00f;
	playerSpaceArray[1] = +1.00f;
	playerSpaceArray[3] = +1.00f;

	playerSpaceArray[6] = -1.00f + offset;
	playerSpaceArray[7] = +1.00f;
	playerSpaceArray[10] = +1.00f;

	playerSpaceArray[12] = -1.00f;
	playerSpaceArray[13] = +1.00f - offset;
	playerSpaceArray[17] = +1.00f;

	return playerSpaceArray;
}

GLfloat* createBombArray(int gameboard) {
	//GLfloat offset;
	GLfloat* bombSpaceArray;

	bombSpaceArray = (GLfloat*)calloc(6, sizeof(GLfloat));

	bombSpaceArray[0] = -1.00f + offset;	//Bottom Right X
	bombSpaceArray[1] = +1.00f - offset;	//Bottom Right Y
	bombSpaceArray[2] = -1.00f;				//Bottom Left X
	bombSpaceArray[3] = +1.00f - offset;	//Bottom Left Y
	bombSpaceArray[4] = -1.00f + offset;	//Top Right X
	bombSpaceArray[5] = +1.00f;						//Top Right Y
	
	return bombSpaceArray;
}

GLfloat* createendSpaceArray(int gameboard) {
	//GLfloat offset;
	GLfloat* endSpaceArray;

	endSpaceArray = (GLfloat*)calloc(18, sizeof(GLfloat));
	//offset = 2.0 / gameboard;

	endSpaceArray[0] = +1.00f;
	endSpaceArray[1] = -1.00f;
	endSpaceArray[3] = +1.00f;

	endSpaceArray[6] = +1.00f - offset;
	endSpaceArray[7] = -1.00f;
	endSpaceArray[10] = +1.00f;

	endSpaceArray[12] = +1.00f; //- (offset / 2);
	endSpaceArray[13] = -1.00f + offset;
	endSpaceArray[17] = +1.00f;



	return endSpaceArray;
}

GLuint*  createboardIndices(int gameboard) {

	int blackSquareTotal;
	GLuint* indicesArray;
	int indicesArraySize;


	if (gameboard % 2 == 0) {
		blackSquareTotal = (gameboard * gameboard) / 2;
	}
	else {
		blackSquareTotal = ((gameboard * gameboard) + 1) / 2;
	}

	indicesArraySize = blackSquareTotal * 6;
	indicesArray = (GLuint*)calloc(indicesArraySize, sizeof(GLuint));

	for (int i = 0; i < blackSquareTotal; i++) {

		indicesArray[0 + (i * 6)] = 0 + (4 * i);
		indicesArray[1 + (i * 6)] = indicesArray[0 + (i * 6)] + 1;
		indicesArray[2 + (i * 6)] = indicesArray[0 + (i * 6)] + 2;
		indicesArray[3 + (i * 6)] = indicesArray[1 + (i * 6)];
		indicesArray[4 + (i * 6)] = indicesArray[2 + (i * 6)] + 1;
		indicesArray[5 + (i * 6)] = indicesArray[2 + (i * 6)];
	}

	return indicesArray;
}




GLfloat* createboardVertArray(int gameboard) {

	int blackSquareTotal;
	int blacksquarVertArraySize;
	GLfloat* blacksquarVertArray;
	//GLfloat offset;
	GLfloat blackSquareoffset;
	int blackSquaresPerRow;

	if (gameboard % 2 == 0) {
		blackSquareTotal = (gameboard * gameboard) / 2;
	}
	else {
		blackSquareTotal = ((gameboard * gameboard) + 1) / 2;
	}

	blacksquarVertArraySize = blackSquareTotal * 8;
	blacksquarVertArray = (GLfloat*)calloc(blacksquarVertArraySize, sizeof(GLfloat));
	printf("\n The array size is %d\n", blacksquarVertArraySize);

	//offset = 2.0 / gameboard;
	blackSquareoffset = offset * 2;

	if (gameboard % 2 == 0) {
		blackSquaresPerRow = gameboard / 2;


		int l = 1;
		for (int i = 0; i < gameboard; i++) {
			if (i == 0) { //STARTING ROW
				for (int j = 0; j < blackSquaresPerRow; j++) {
					if (j == 0) {
						blacksquarVertArray[0] = -1.00f;	//ALWAYS THIS
						blacksquarVertArray[1] = +1.00f;	//ALWAYS THIS
						blacksquarVertArray[2] = blacksquarVertArray[0] + offset;
						blacksquarVertArray[3] = blacksquarVertArray[1];
						blacksquarVertArray[4] = blacksquarVertArray[0];
						blacksquarVertArray[5] = blacksquarVertArray[1] - offset;
						blacksquarVertArray[6] = blacksquarVertArray[2];
						blacksquarVertArray[7] = blacksquarVertArray[5];
						for (int k = 1; k < blackSquaresPerRow; k++) {

							int position = l * 8;
							blacksquarVertArray[position + 0] = blacksquarVertArray[0] + (blackSquareoffset * l);
							blacksquarVertArray[position + 1] = blacksquarVertArray[1];
							blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
							blacksquarVertArray[position + 3] = blacksquarVertArray[1];
							blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
							blacksquarVertArray[position + 5] = blacksquarVertArray[1] - offset;
							blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
							blacksquarVertArray[position + 7] = blacksquarVertArray[1] - offset;
							l++;
						}
					}
					l = 1;
				}
			}
			else if (i % 2 != 0) { //ODD ROW 
				int index = i * (8 * blackSquaresPerRow);
				blacksquarVertArray[index + 0] = -1.00f + offset;
				blacksquarVertArray[index + 1] = +1.00f - (offset * i);
				blacksquarVertArray[index + 2] = blacksquarVertArray[index + 0] + offset;
				blacksquarVertArray[index + 3] = blacksquarVertArray[index + 1];
				blacksquarVertArray[index + 4] = blacksquarVertArray[index + 0];
				blacksquarVertArray[index + 5] = blacksquarVertArray[index + 1] - offset;
				blacksquarVertArray[index + 6] = blacksquarVertArray[index + 2];
				blacksquarVertArray[index + 7] = blacksquarVertArray[index + 5];
				for (int k = 1; k < blackSquaresPerRow; k++) {
					int position = index + (8 * l);
					blacksquarVertArray[position + 0] = blacksquarVertArray[index + 0] + (blackSquareoffset * l);
					blacksquarVertArray[position + 1] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
					blacksquarVertArray[position + 3] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
					blacksquarVertArray[position + 5] = blacksquarVertArray[index + 1] - offset;
					blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
					blacksquarVertArray[position + 7] = blacksquarVertArray[index + 1] - offset;
					l++;
				}
				l = 1;
			}
			else { //EVEN ROW
				int index = i * (8 * blackSquaresPerRow);
				blacksquarVertArray[index + 0] = blacksquarVertArray[0];
				blacksquarVertArray[index + 1] = +1.00f - (offset * i);
				blacksquarVertArray[index + 2] = blacksquarVertArray[index + 0] + offset;
				blacksquarVertArray[index + 3] = blacksquarVertArray[index + 1];
				blacksquarVertArray[index + 4] = blacksquarVertArray[index + 0];
				blacksquarVertArray[index + 5] = blacksquarVertArray[index + 1] - offset;
				blacksquarVertArray[index + 6] = blacksquarVertArray[index + 2];
				blacksquarVertArray[index + 7] = blacksquarVertArray[index + 5];
				for (int k = 1; k < blackSquaresPerRow; k++) {
					int position = index + (8 * l);
					blacksquarVertArray[position + 0] = blacksquarVertArray[index + 0] + (blackSquareoffset * l);
					blacksquarVertArray[position + 1] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
					blacksquarVertArray[position + 3] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
					blacksquarVertArray[position + 5] = blacksquarVertArray[index + 1] - offset;
					blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
					blacksquarVertArray[position + 7] = blacksquarVertArray[index + 1] - offset;
					l++;
				}
				l = 1;
			}
		}
	}

	if (gameboard % 2 != 0) {
		blackSquaresPerRow = gameboard / 2;
		int blackSquaresPerRowOdd = blackSquaresPerRow + 1;

		int l = 1;
		for (int i = 0; i < gameboard; i++) {
			if (i == 0) { //STARTING ROW
				for (int j = 0; j < blackSquaresPerRow; j++) {
					if (j == 0) {
						blacksquarVertArray[0] = -1.00f;	//ALWAYS THIS
						blacksquarVertArray[1] = +1.00f;	//ALWAYS THIS
						blacksquarVertArray[2] = blacksquarVertArray[0] + offset;
						blacksquarVertArray[3] = blacksquarVertArray[1];
						blacksquarVertArray[4] = blacksquarVertArray[0];
						blacksquarVertArray[5] = blacksquarVertArray[1] - offset;
						blacksquarVertArray[6] = blacksquarVertArray[2];
						blacksquarVertArray[7] = blacksquarVertArray[5];
						for (int k = 1; k < blackSquaresPerRow + 1; k++) {

							int position = l * 8;
							blacksquarVertArray[position + 0] = blacksquarVertArray[0] + (blackSquareoffset * l);
							blacksquarVertArray[position + 1] = blacksquarVertArray[1];
							blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
							blacksquarVertArray[position + 3] = blacksquarVertArray[1];
							blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
							blacksquarVertArray[position + 5] = blacksquarVertArray[1] - offset;
							blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
							blacksquarVertArray[position + 7] = blacksquarVertArray[1] - offset;
							l++;
						}
					}
					l = 1;
				}
			}
			else if (i % 2 != 0) { //ODD ROW 
				int index = i * (8 * blackSquaresPerRow);
				blacksquarVertArray[index + 0] = -1.00f + offset;
				blacksquarVertArray[index + 1] = +1.00f - (offset * i);
				blacksquarVertArray[index + 2] = blacksquarVertArray[index + 0] + offset;
				blacksquarVertArray[index + 3] = blacksquarVertArray[index + 1];
				blacksquarVertArray[index + 4] = blacksquarVertArray[index + 0];
				blacksquarVertArray[index + 5] = blacksquarVertArray[index + 1] - offset;
				blacksquarVertArray[index + 6] = blacksquarVertArray[index + 2];
				blacksquarVertArray[index + 7] = blacksquarVertArray[index + 5];

				for (int k = 0; k < blackSquaresPerRow; k++) {
					int position = index + (8 * l);
					blacksquarVertArray[position + 0] = blacksquarVertArray[index + 0] + (blackSquareoffset * l);
					blacksquarVertArray[position + 1] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
					blacksquarVertArray[position + 3] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
					blacksquarVertArray[position + 5] = blacksquarVertArray[index + 1] - offset;
					blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
					blacksquarVertArray[position + 7] = blacksquarVertArray[index + 1] - offset;
					l++;
				}
				l = 1;
			}
			else { //EVEN ROW
				int index = i * (8 * blackSquaresPerRow);
				blacksquarVertArray[index + 0] = blacksquarVertArray[0];
				blacksquarVertArray[index + 1] = +1.00f - (offset * i);
				blacksquarVertArray[index + 2] = blacksquarVertArray[index + 0] + offset;
				blacksquarVertArray[index + 3] = blacksquarVertArray[index + 1];
				blacksquarVertArray[index + 4] = blacksquarVertArray[index + 0];
				blacksquarVertArray[index + 5] = blacksquarVertArray[index + 1] - offset;
				blacksquarVertArray[index + 6] = blacksquarVertArray[index + 2];
				blacksquarVertArray[index + 7] = blacksquarVertArray[index + 5];
				for (int k = 1; k < blackSquaresPerRow; k++) {
					int position = index + (8 * l);
					blacksquarVertArray[position + 0] = blacksquarVertArray[index + 0] + (blackSquareoffset * l);
					blacksquarVertArray[position + 1] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 2] = blacksquarVertArray[position + 0] + offset;
					blacksquarVertArray[position + 3] = blacksquarVertArray[index + 1];
					blacksquarVertArray[position + 4] = blacksquarVertArray[position + 0];
					blacksquarVertArray[position + 5] = blacksquarVertArray[index + 1] - offset;
					blacksquarVertArray[position + 6] = blacksquarVertArray[position + 2];
					blacksquarVertArray[position + 7] = blacksquarVertArray[index + 1] - offset;
					l++;
				}
				l = 1;
			}
		}

		int index = 0;
		int remainingSquares = gameboard / 2;
		for (int i = 0; i < blacksquarVertArraySize; i++) {
			if (blacksquarVertArray[i] != 0.00f) {
				index++;
			}
			else {
				break;
			}
		}

		for (int i = 0; i < blackSquaresPerRowOdd; i++) {
			blacksquarVertArray[index + (8 * i) + 0] = +1.00f - offset;						//Top Left X
			blacksquarVertArray[index + (8 * i) + 1] = +1.00f - (blackSquareoffset * i);
			blacksquarVertArray[index + (8 * i) + 2] = +1.00f;								//Top Right X
			blacksquarVertArray[index + (8 * i) + 3] = +1.00f - (blackSquareoffset * i);
			blacksquarVertArray[index + (8 * i) + 4] = blacksquarVertArray[index + (8 * i) + 0];				//BottomLeft X
			blacksquarVertArray[index + (8 * i) + 5] = blacksquarVertArray[index + (8 * i) + 1] - offset;
			blacksquarVertArray[index + (8 * i) + 6] = blacksquarVertArray[index + (8 * i) + 2];						//BottomRight X
			blacksquarVertArray[index + (8 * i) + 7] = blacksquarVertArray[index + (8 * i) + 3] - offset;
		}

	}




	printf("\n\n There are %d black squares in a %d x %d gameboard\n", blackSquareTotal, gameboard, gameboard);
	printf("\n The offset is %f", offset);


	return blacksquarVertArray;
}

void displayStats(void) {
	printf("\n\n Score: %d Moves Made. ", moves);
	printf("Lives Left: %d", lives);
}

void push(TMoves** list, int value) {
	TMoves* newList = (TMoves*)malloc(sizeof(TMoves));
	newList->lastPosition = value;
	newList->lastMove = *list;

	*list = newList;
}

int pop(TMoves** listPtr) {
	int value = -1;

	if ((*listPtr)->lastPosition != -1) {
		value = (*listPtr)->lastPosition;
		TMoves* tempPtr = (*listPtr)->lastMove;

		free(*listPtr);

		*listPtr = tempPtr;
	}
	return value;
}

void printMoves(TMoves* list) {
	if (list != 0) {
		printf("\n%d", list->lastPosition);
		printMoves(list->lastMove);
	}
}

void printBoard(int** board, int boardSize) {
	for (int i = 0; i < boardSize; i++) {
		for (int j = 0; j < boardSize; j++) {
			printf("%d ", *(*(board + i) + j));
		}
		printf("\n");
	}
}

void moveLeft() {
	//printf("\nleft\n");
	//printf("\n\nBEFORE: This is the leftrightpress value: %d", leftrightpress);
	leftrightpress = ((leftrightpress > 0) ? leftrightpress - 1 : leftrightpress);
	//printf("\nAFTER: This is the leftrightpress value: %d", leftrightpress);
	moves++;
	displayStats();

	if (currentCol > 0) {
		(*(*(board + currentRow) + currentCol))--;
		(*(*(board + currentRow) + currentCol - 1))++;
		currentCol--;
		push(&movesList, 1);
	}

	printf("\n\n");
	printBoard(board, gameboard);
}

void moveRight() {
	//printf("\nright");
	//printf("\n\nBEFORE: This is the leftrightpress value: %d", leftrightpress);
	leftrightpress = ((leftrightpress < gameboard - 1) ? leftrightpress + 1 : leftrightpress);
	//printf("\nAFTER: This is the leftrightpress value: %d", leftrightpress);
	moves++;
	displayStats();

	if (currentCol < gameboard - 1) {
		(*(*(board + currentRow) + currentCol))--;
		(*(*(board + currentRow) + currentCol + 1))++;
		currentCol++;
		push(&movesList, 2);
	}

	printf("\n\n");
	printBoard(board, gameboard);
}

void moveUp() {
	//printf("\nup");
	//printf("\n\nBEFORE: This is the updownpress value: %d", updownpress);
	updownpress = ((updownpress >  0) ? updownpress - 1 : updownpress);
	//printf("\nAFTER: This is the updownpress value: %d", updownpress);
	moves++;
	displayStats();

	//Answer Array
	if (currentRow > 0) {
		(*(*(board + currentRow) + currentCol))--;
		(*(*(board + currentRow - 1) + currentCol))++;
		currentRow--;
		push(&movesList, 3);
	}

	printf("\n\n");
	printBoard(board, gameboard);
}

void moveDown() {
	//printf("\ndown");
	//printf("\n\nBEFORE: This is the updownpress value: %d", updownpress);
	//updownpress--;
	updownpress = ((updownpress > -(gameboard - 1)) ? updownpress + 1 : updownpress);
	//printf("\nAFTER: This is the updownpress value: %d", updownpress);
	moves++;
	displayStats();

	if (currentRow < gameboard - 1) {
		(*(*(board + currentRow) + currentCol))--;
		(*(*(board + currentRow + 1) + currentCol))++;
		currentRow++;
		push(&movesList, 4);
	}

	printf("\n\n");
	printBoard(board, gameboard);
}

void checkBombs(int Xposition, int Yposition) {

	if (*(*(board + Yposition) + Xposition) == 3) {
		lives--;
		printf("\n YOU LOST A LIFE");
		*(*(board + Yposition) + Xposition) = 2;
	}

	
}