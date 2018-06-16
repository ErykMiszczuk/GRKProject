#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model fishModel;
obj::Model bottomPlaneModel;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureFish;
GLuint textureShip;
GLuint textureBottomPlane;
GLuint textureSink;

glm::vec3 fishVectors[10];

const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;
int NUM_FISH = 10;

unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int rbo;
unsigned int quadVAO, quadVBO;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
						 // positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};

float poprzedniaPozycjaMyszki[2] = { 0,0 };
float roznicaMyszki[2] = { 0,0 };

void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	float nowaPozycja[2] = { x, y };
	roznicaMyszki[0] = poprzedniaPozycjaMyszki[0] - nowaPozycja[0];
	roznicaMyszki[1] = poprzedniaPozycjaMyszki[1] - nowaPozycja[1];
	poprzedniaPozycjaMyszki[0] = nowaPozycja[0];
	poprzedniaPozycjaMyszki[1] = nowaPozycja[1];
}

glm::mat4 createCameraMatrix()
{
	glm::quat obrotY = glm::angleAxis(roznicaMyszki[0] * 0.01f, glm::vec3(0, 1, 0));
	glm::quat obrotX = glm::angleAxis(roznicaMyszki[1] * 0.01f, glm::vec3(1, 0, 0));

	glm::quat rotationChange = obrotX * obrotY;

	rotation = rotationChange * rotation;
	rotation = glm::normalize(rotation);

	roznicaMyszki[0] = 0;
	roznicaMyszki[1] = 0;

	cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);

	return Core::createViewMatrixQuat(cameraPos, rotation);
	/*
	cameraDir = glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0.0f, sinf(cameraAngle - glm::radians(90.0f)));
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
	*/
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void renderScene()
{


	//// first pass
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
	//glEnable(GL_DEPTH_TEST);



	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 shipRotation = glm::mat4_cast(glm::inverse(rotation));

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.25f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * shipRotation * shipInitialTransformation;



	//glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);




	//drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));

	drawObjectTexture(&shipModel, shipModelMatrix, textureShip);

	glm::mat4 sink = glm::translate(glm::vec3(0, -2.5f, 0)) * glm::rotate(glm::radians(100.0f), glm::vec3(1, 1, 0)) * glm::scale(glm::vec3(2.0f));

	drawObjectTexture(&shipModel, sink, textureSink);

	int iterm = -108;
	int loopi = 0;
	for (loopi = 0; loopi < 3; loopi++) {
		drawObjectTexture(&bottomPlaneModel, glm::translate(glm::vec3(-108, -5, iterm)), textureBottomPlane);
		drawObjectTexture(&bottomPlaneModel, glm::translate(glm::vec3(0, -5, iterm)), textureBottomPlane);
		drawObjectTexture(&bottomPlaneModel, glm::translate(glm::vec3(108, -5, iterm)), textureBottomPlane);
		iterm += 108;
	}

	//drawObjectTexture(&bottomPlaneModel, glm::translate(glm::vec3(0, -15, 0)), textureBottomPlane);

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	for (int i = 0; i < 10; i++) {

		glm::mat4 translationToRotiationOrigin = glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.0f));


		glm::mat4 rotation =
		{
			cos(time),0,sin(time),0,
			0,1,0,0,
			-sin(time),0,cos(time),0,
			0,0,0,1
		};



		/*glm::mat4 rotation =
		{
			sin(time),0,0,0,
			0,1,0,0,
			0,0,cos(time),0,
			0,0,0,1
		};*/



		glm::vec3 neg;
		neg.x = -fishVectors[i].x;
		neg.y = -fishVectors[i].y;
		neg.z = fishVectors[i].z;


		glm::mat4 aft = translationToRotiationOrigin * rotation * glm::translate(neg);


		drawObjectTexture(&fishModel, aft, textureFish);
	}



	//screenShader.use();
	//glBindVertexArray(quadVAO);
	//glDisable(GL_DEPTH_TEST);
	//glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	//glDrawArrays(GL_TRIANGLES, 0, 6);




	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEBUG_OUTPUT);






	//glGenVertexArrays(1, &quadVAO);
	//glGenBuffers(1, &quadVBO);
	//glBindVertexArray(quadVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));







	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	fishModel = obj::loadModelFromFile("models/ryba.obj");
	shipModel = obj::loadModelFromFile("models/OrcaSub1.obj");
	textureShip = Core::LoadTexture("textures/orca_sub_red.png");
	textureSink = Core::LoadTexture("textures/orca_sub_sink.png");
	//textureShip = Core::GenerateTexture();
	textureFish = Core::LoadTexture("textures/rybauvmap.png");
	//textureAsteroid = Core::GenerateTexture();
	bottomPlaneModel = obj::loadModelFromFile("models/bottom1.obj");
	textureBottomPlane = Core::GenerateTexture(100, 100, 30, 1);

	static const float astRadius = 15;
	for (int i = 0; i < 10; i++) {
		float angle = (float(i))*(2 * glm::pi<float>() / NUM_FISH);
		fishVectors[i] = glm::vec3(cosf(angle), 0.0f, sinf(angle)) * astRadius;
	}


	//glGenFramebuffers(1, &framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	//// create a color attachment texture
	//glGenTextures(1, &textureColorbuffer);
	//glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	//// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	//glGenRenderbuffers(1, &rbo);
	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	//																							  // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	//	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	//}
	//else {
	//	std::cout << "RenderBuffer Complete \n";
	//}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}