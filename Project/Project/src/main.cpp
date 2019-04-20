#include <iostream>
#define _USE_MATH_DEFINES //use to access M_PI
#include "math.h"
#include "glad/glad.h"

//#define TINYOBJLOADER_IMPLEMENTATION
//#include "tiny_obj_loader.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "BaseCode/GLSL.h"
#include "BaseCode/Program.h"
#include "BaseCode/Shape.h"
#include "BaseCode/WindowManager.h"
#include "BaseCode/GLTextureWriter.h"
#include "BaseCode/MatrixStack.h"
#include "GameObject.h" // Our Game Object Class
#include "ourCoreFuncs.h"


#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

//Globals

//---
int p1Collisions = 0;

//--- Variables for players bbox 
GLuint p1_vbo_vertices;
GLuint p1_ibo_elements;
vec3 p1_bboxSize, p1_bboxCenter;
mat4 p1_bboxTransform;

//--- Vector of all actor game objects
vector<shared_ptr<GameObject> > sceneActorGameObjs;

//Camera Timing
float deltaTime = 0.0f, lastTime = glfwGetTime();
float lastFrame = 0.0f;
int nbFrames = 0;
float elapsedTime = 0.0f;


//Camera:
vec3 eye = vec3(0, 0.5, 0); //was originally 0,0,0
vec3 up = vec3(0, 1, 0);

vec3 center;
//const vec3 movespd = vec3(.2);	// movespd for each keypress. equivalent to .2, .2, .2

//Animation:
float orbRotate = 0.0;
float smallRotate = 0.0;
float bunnyRotate = 0.0;


class Application : public EventCallbacks
{

public:

	// Public variables
	float theta = 0;
	float phi = 0;
	float radius = 1;
	float x, y, z;
	const float to_radians = M_PI / 180;

	//Time variable which determines how often bunnies spawn;
	float bunSpawn = 4.0f; // float P;
	float bunSpawnReset = bunSpawn; //So we dont need magic number when resetting bunSpawn
	int bunbunCounter = 3;

	//Mouse info
	int tempX = 0;
	int tempY = 0;

	//More Camera Info
	vec3 camMove;

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;

	// Access OBJ files
	shared_ptr<Shape> bunnyShape;
	shared_ptr<Shape> cube;
	shared_ptr<Shape> sphere;

	shared_ptr<GameObject> bunBun, groundbox, bunBunTwo;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	// Contains vertex information for OpenGL
	GLuint CylVertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint CylVertexBufferID;

	// bool FirstTime = true;


	float cTheta = 0;
	bool mouseDown = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{

		float movespd = 20.0f * deltaTime;

		if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//Keys to control the camera movement
		else if (key == GLFW_KEY_W)
		{
			center = center + (camMove * movespd);
			eye = eye + (camMove * movespd);

			if (eye.y <= 0)
			{
				eye.y = 0;
			}

		}
		else if (key == GLFW_KEY_A)
		{
			//Left
			center += cross(up, camMove) * movespd;
			eye += cross(up, camMove) * movespd;

		}
		else if (key == GLFW_KEY_D)
		{
			//Right
			center -= cross(up, camMove) * movespd;
			eye -= cross(up, camMove) * movespd;

		}
		else if (key == GLFW_KEY_S)
		{
			center = center - (movespd * camMove);
			//Backward
			eye = eye - (camMove * movespd);

			if (eye.y <= 0)
			{
				eye.y = 0;
			}

		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		return;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}

		if (action == GLFW_RELEASE)
		{
			mouseDown = false;
		}
	}


	void cursorPosCallback(GLFWwindow* window, double posX, double posY)
	{
		double newX, newY;

		//Calculate the new cursor coordinates based on the difference in position from last time to now
		newX = posX - tempX;
		newY = posY - tempY;

		//set up the input mode
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Update pitch and yaw
		phi = phi + (-newY / 400);
		theta = theta + (newX / 400);

		// Limit the angle of V up and down
		if (phi > 80 * to_radians)
		{
			phi = 80 * to_radians; // restrict to 80 degrees
		}
		else if (phi < -80 * to_radians)
		{
			phi = -(80 * to_radians);
		}

		//Update the coordinates of the temps
		tempY = tempY + newY;
		tempX = tempX + newX;

	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	//Function From Program 3 webpage, case 3 is my implementation for prob 2b
	void SetMaterial(int i) {
		switch (i) {
		case 0: // shiny blue plastic
			glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
			glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
			glUniform1f(prog->getUniform("shine"), 120.0);
			break;
		case 1: // flat grey
			glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
			glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
			glUniform1f(prog->getUniform("shine"), 4.0);
			break;
		case 2: // brass
			glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
			glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
			glUniform1f(prog->getUniform("shine"), 27.9);
			break;
		case 3: //Mine: wood
			glUniform3f(prog->getUniform("MatAmb"), 0.15, 0.17, 0.12);
			glUniform3f(prog->getUniform("MatDif"), 0.83, 0.5, 0.2);
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.22, 0.22);
			glUniform1f(prog->getUniform("shine"), 20.0);
			break;
		}
	}

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.02f, .02f, .02f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("V");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addUniform("lightSource"); //lighting uniform
		prog->addUniform("hit"); //Uniform for determining color based on hit or not

		//create two frame buffer objects to toggle between
		glGenFramebuffers(2, frameBuf);
		glGenTextures(2, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

		//more FBO set up
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		//create another FBO so we can swap back and forth
		createFBO(frameBuf[1], texBuf[1]);
		//this one doesn't need depth

		//set up the shaders to blur the FBO just a placeholder pass thru now
		//next lab modify and possibly add other shaders to complete blur
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(
			resourceDirectory + "/mirror_vert.glsl",
			resourceDirectory + "/mirror_frag.glsl");
		if (!texProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		texProg->addUniform("tex");
		texProg->addAttribute("vertPos");
		texProg->addUniform("dir");
		texProg->addUniform("P");
		texProg->addUniform("M");
		texProg->addUniform("V");
	}
	
	void initPlayerBbox()
	{
		// Cube 1x1x1, centered on origin
		GLfloat p1_vertices[] = {
		  -0.5, -0.5, -0.5, 1.0,
		   0.5, -0.5, -0.5, 1.0,
		   0.5,  0.5, -0.5, 1.0,
		  -0.5,  0.5, -0.5, 1.0,
		  -0.5, -0.5,  0.5, 1.0,
		   0.5, -0.5,  0.5, 1.0,
		   0.5,  0.5,  0.5, 1.0,
		  -0.5,  0.5,  0.5, 1.0,
		};
		//GLuint vbo_vertices;
		glGenBuffers(1, &p1_vbo_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, p1_vbo_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(p1_vertices), p1_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind

		GLushort p1_elements[] = {
		  0, 1, 2, 3,
		  4, 5, 6, 7,
		  0, 4, 1, 5, 2, 6, 3, 7
		};
		//GLuint ibo_elements;
		glGenBuffers(1, &p1_ibo_elements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p1_ibo_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(p1_elements), p1_elements, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind


		p1_bboxSize = glm::vec3(3.0f);
		p1_bboxCenter = glm::vec3(3.f/2.f);
		p1_bboxTransform = glm::translate(glm::mat4(1), p1_bboxCenter) * glm::scale(glm::mat4(1), p1_bboxSize);

	}

	void initGeom(const std::string& resourceDirectory)
	{

		//Initialize the geometry to render a quad to the screen
		initQuad();

		// Initialize the bunny obj mesh VBOs etc
		bunnyShape = make_shared<Shape>();
		bunnyShape->loadMesh(resourceDirectory + "/bunny.obj");
		bunnyShape->resize();
		bunnyShape->init();

		// Initialize the cube OBJ model
		cube = make_shared<Shape>();
		cube->loadMesh(resourceDirectory + "/cube.obj");
		cube->resize();
		cube->init();


		// Initialize the sphere OBJ model
		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();

		// Setup player bbox
		initPlayerBbox();


		// Setup new Ground plane
		glm::vec3 position = glm::vec3(0.0f);
		float velocity = 0.0f;
		glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		groundbox = make_shared<GameObject>("groundbox", cube, resourceDirectory, prog, position, velocity, orientation, false);


	}

	/**** geometry set up for a quad *****/
	void initQuad()
	{
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	}

	/* Helper function to create the framebuffer object and
		associated texture to write to */
	void createFBO(GLuint& fb, GLuint& tex)
	{
		//initialize FBO
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}


	void renderPlayerBbox(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P)
	{
		prog->bind();
		// Recompute the position of the box b4 drawing it
		p1_bboxCenter = center;
		p1_bboxTransform = translate(glm::mat4(1), p1_bboxCenter) * glm::scale(glm::mat4(1), p1_bboxSize);

		//Check Collisions
		for(int i = 0; i < sceneActorGameObjs.size(); i++){
			bool collisionX = p1_bboxCenter.x + p1_bboxSize.x >= sceneActorGameObjs[i]->bboxCenter.x && sceneActorGameObjs[i]->bboxCenter.x + sceneActorGameObjs[i]->bboxSize.x >= p1_bboxCenter.x;
			bool collisionY = p1_bboxCenter.y + p1_bboxSize.y >= sceneActorGameObjs[i]->bboxCenter.y && sceneActorGameObjs[i]->bboxCenter.y + sceneActorGameObjs[i]->bboxSize.y >= p1_bboxCenter.y;
			bool collisionZ = p1_bboxCenter.z + p1_bboxSize.z >= sceneActorGameObjs[i]->bboxCenter.z && sceneActorGameObjs[i]->bboxCenter.z + sceneActorGameObjs[i]->bboxSize.z >= p1_bboxCenter.z;

			// If there is a collision with a moving object
			if( !(sceneActorGameObjs[i]->hitByPlayer) && collisionX && collisionY && collisionZ){
				printf("Camera Collision\n");
				sceneActorGameObjs[i]->velocity = 0.0f; // Stop the object you collide with
				p1Collisions += 1; // Increment the number of objects the player has collided with
				sceneActorGameObjs[i]->hitByPlayer = true;
			}

			//return collisionX && collisionY && collisionZ;
		}

		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(p1_bboxTransform));
		glBindBuffer(GL_ARRAY_BUFFER, p1_vbo_vertices);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,  // attribute
			4,                  // number of elements per vertex, here (x,y,z,w)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p1_ibo_elements);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		prog->unbind();
	}

	void renderGroundPlane(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, int surveillancePOV)
	{
		vec3 camLoc;
		if (surveillancePOV == 1)
		{
			camLoc = vec3(0, .75, 9);

		}
		else  if (surveillancePOV == 0)
		{
			camLoc = eye;
		}

		prog->bind();

		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(0, -1, 0)); //move the plane down a little bit in y space 
		M->scale(vec3(40, .1, 40)); // turn the cube into a plane
		groundbox->step(deltaTime, M, P, camLoc, center, up);
		//add uniforms to shader
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(camLoc, center, up)));
		glUniform3f(prog->getUniform("lightSource"), 0, 88, 10);
		//glUniform3f(prog->getUniform("eye"), 0, 10, 0);
		//Set up the Lighting Uniforms, Copper for this
		SetMaterial(3);
		//draw
		//cube->draw(prog);
		groundbox->DrawGameObj();
		M->popMatrix();

		prog->unbind();

		return;
	}


	void renderWorldBox() {

	}

	bool checkCollisions(shared_ptr<GameObject> objOne, shared_ptr<GameObject> objTwo) {
		bool collisionX = objOne->bboxCenter.x + objOne->bboxSize.x >= objTwo->bboxCenter.x && objTwo->bboxCenter.x + objTwo->bboxSize.x >= objOne->bboxCenter.x;
		bool collisionY = objOne->bboxCenter.y + objOne->bboxSize.y >= objTwo->bboxCenter.y && objTwo->bboxCenter.y + objTwo->bboxSize.y >= objOne->bboxCenter.y;
		bool collisionZ = objOne->bboxCenter.z + objOne->bboxSize.z >= objTwo->bboxCenter.z && objTwo->bboxCenter.z + objTwo->bboxSize.z >= objOne->bboxCenter.z;

		return collisionX && collisionY && collisionZ;
	}

	void checkAllGameObjects()
	{
		
		for (int i = 0; i < sceneActorGameObjs.size(); i++)
		{
			for (int j = i + 1; j < sceneActorGameObjs.size(); j++) {
				bool wasCollision = checkCollisions(sceneActorGameObjs[i], sceneActorGameObjs[j]);

				if (wasCollision) {
					//printf("Collision occured\n");
					sceneActorGameObjs[i]->velocity = 0;
					sceneActorGameObjs[j]->velocity = 0;
				}
			}
		}
	}

	

	void renderBun(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, int surveillancePOV, int offsetX, int offsetZ)
	{
		vec3 camLoc;
		if (surveillancePOV == 1)
		{
			camLoc = vec3(0, .75, 9);

		}
		else  if (surveillancePOV == 0)
		{
			camLoc = eye;
		}

		
		prog->bind(); // Bind the Simple Shader



		for (int i = 0; i < sceneActorGameObjs.size(); i++) {
			M->pushMatrix();
			M->loadIdentity();

			// Update the position of the rabbit based on velocity, time elapsed also updates the center of the bbox
			sceneActorGameObjs[i]->step(deltaTime, M, P, camLoc, center, up);
			// bunBun->DoCollisions()


			//add uniforms to shader
			// Set the materials of the bunny depending on if the player has hit it or not
			if (sceneActorGameObjs[i]->hitByPlayer)
			{
				SetMaterial(2);
				//M->rotate(180.0f, vec3(0, 1, 0));
				// glUniform1f(prog->getUniform("hit"), 1); //old method
			}
			else
			{
				SetMaterial(1);
			}
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(camLoc, center, up)));
			
			glUniform3f(prog->getUniform("lightSource"), 0, 88, 10);
			sceneActorGameObjs[i]->DrawGameObj(); // Draw the bunny model and render bbox

			M->popMatrix();
		}


		prog->unbind(); // Unbind the Simple Shader

		return;

	}


	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Set Delta Time and lastFrame
		float currentFrame = glfwGetTime();
		float currentTime = glfwGetTime();
		nbFrames += 1;
		// At intervals print out information to the console
		if (currentTime - lastTime >= 1.0 ) {
			printf("MS/FPS: %f    FPS: %f\n", 1000.0 / double(nbFrames) , double(nbFrames)); // Print out ms/fps and frame rate
			printf("Objects on the ground: %d\n", sceneActorGameObjs.size()); // Print out the number of objects in the game
			printf("Objects Collided with %d\n\n", p1Collisions);
			nbFrames = 0;
			lastTime += 1.0;
		}

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		elapsedTime += deltaTime;


		//--TODO: Move maybe. Spawns random bunnies
		bunSpawn -= deltaTime;
		if (bunSpawn <= 0 && (sceneActorGameObjs.size() < 40)) // Spawn a bunny if the timer is up and there are less than 25 in the scene
		{
			printf("spawn bunny\n");
			bunSpawn = bunSpawnReset;
			//Template for random float
			//float r3 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
			float randX = -30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f - -30.0f)));
			float randZ = -30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f - -30.0f)));
			float randVel = 1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (8.0f - 1.0f)));
			
			string BunName = "bunBun" + to_string(bunbunCounter);

			vec3 position = vec3(randX, 0.0f, randZ);

			vec3 orientation = vec3(0.0f, 0.0f, 1.0f);

			shared_ptr<GameObject> newBunny = make_shared<GameObject>(BunName, bunnyShape, "resources/", prog, position, randVel, orientation, true);
			sceneActorGameObjs.push_back(newBunny);

		}

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width / (float)height;

		// Setup yaw and pitch of camera for lookAt()
		x = radius * cos(phi)*cos(theta);
		y = radius * sin(phi);
		z = radius * cos(phi)*sin(theta);
		center = eye + vec3(x, y, z);
		camMove = vec3(x, y, z);

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto M = make_shared<MatrixStack>();


		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);


		/*Draw the actual scene*/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		M->pushMatrix(); // Matrix for the Scene
		renderGroundPlane(M, P, 0); //draw the ground plane

		// Update the position of the players bbox
		renderPlayerBbox(M, P);

		//renderAnimSphere(M, P, 0, 0, 0); //draw the hierarchical modeled animated spheres

		M->pushMatrix();
		//checkAllGameObjects();
		renderBun(M, P, 0, 0, 0);
		checkAllGameObjects();
		//bunBun->DoCollisions(groundbox);
		M->popMatrix();


		M->popMatrix(); // Pop Scene Matrix

	}

};


int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "resources/";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1280, 720); //was 512,512.
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);


	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();


		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());

		// Poll for and process events.
		glfwPollEvents();

		//update spinning Orbs
		if (orbRotate == 360.0)
		{
			orbRotate = 0.0;

		}
		else
		{
			orbRotate += 2.0;
		}
		if (smallRotate == 360.0)
		{
			smallRotate = 0.0;

		}
		else
		{
			smallRotate += 0.1;
		}

		//update spinning Bunnies

		if (bunnyRotate == 360.0)
		{
			bunnyRotate = 0.0;

		}
		else
		{
			bunnyRotate += 0.025;
		}

	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
