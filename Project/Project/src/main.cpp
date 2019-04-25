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

// -- Variables for possessed actor
shared_ptr<GameObject> possessedActor = NULL;

//--- Variables for players bbox 
GLuint p1_vbo_vertices;
GLuint p1_ibo_elements;
vec3 p1_bboxSize, p1_bboxCenter;
mat4 p1_bboxTransform;

//--- Vector of all actor game objects plus arrays of player units and enemy units
vector<shared_ptr<GameObject> > sceneActorGameObjs;
vector<shared_ptr<GameObject> > robotUnits;
vector<shared_ptr<GameObject> > alienUnits;

//Camera Timing
float deltaTime = 0.0f, lastTime = glfwGetTime();
float lastFrame = 0.0f;
int nbFrames = 0;
float elapsedTime = 0.0f;

bool isCaptureCursor = false;

//----- Camera Variables
bool isOverheadView = true; // If camera is in Overhead view or follow view, currently overhead view is broken
bool camUpdate = false;
// Possession Camera
vec3 pCamEye = vec3(0, 10, 10); //was originally 0,0,0
vec3 up = vec3(0, 1, 0);
//vec3 pCamCenter = vec3(0.f, 0.f, 0.f);
//const vec3 movespd = vec3(.2);	// movespd for each keypress. equivalent to .2, .2, .2

// Properties for Overhead Camera
//vec3 oCamCenter = vec3(-33.9f, 51.884197f, 0.059f);
vec3 oCamEye = vec3(-17.4f, 40.0f, 5.00);

// Current Camera
vec3 curCamEye = oCamEye;
vec3 curCamCenter;
// ------------------------

//Animation:
float orbRotate = 0.0;
float smallRotate = 0.0;
float bunnyRotate = 0.0;

// Class imported from Connor's time in CSC 474
//class camera
//{
//public:
//	glm::vec3 pos, rot;
//	int w, a, s, d;
//	camera()
//	{
//		w = a = s = d = 0;
//		pos = rot = glm::vec3(0, 0, 0);
//	}
//	glm::mat4 process(double ftime)
//	{
//		float speed = 0;
//		if (w == 1)
//		{
//			speed = 10 * ftime;
//		}
//		else if (s == 1)
//		{
//			speed = -10 * ftime;
//		}
//		float yangle = 0;
//		if (a == 1)
//			yangle = -3 * ftime;
//		else if (d == 1)
//			yangle = 3 * ftime;
//		rot.y += yangle;
//		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
//		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
//		dir = dir * R;
//		pos += glm::vec3(dir.x, dir.y, dir.z);
//		glm::mat4 T = glm::translate(glm::mat4(1), pos);
//		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0)); //Tilt the camera on x-axis based on rot.x, set in initGeom
//		return Rx * R * T;
//	}
//};
class Application : public EventCallbacks
{

public:

	// Public variables
	float theta = 0;
	float phi = 0;
	float radius = 1;
	float x, y, z, ox, oy, oz;
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

	//Assign who's turn it is. TODO: Make random at some point(?) for now start at 1
	int whoseTurn = 1;

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;

	// Access OBJ files
	shared_ptr<Shape> bunnyShape, maRobotShape;
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

		float followMoveSpd = 40.0f * deltaTime;
		float overheadMoveSpd = 50.0f * deltaTime;

		if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//Keys to control the camera movement
		if (!camUpdate && !isOverheadView && (possessedActor == NULL))
		{
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				curCamCenter = curCamCenter + (camMove * followMoveSpd);
				curCamEye = curCamEye + (camMove * followMoveSpd);

				if (curCamEye.y <= 0)
				{
					curCamEye.y = 0;
				}

			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//Left
				curCamCenter += cross(up, camMove) * followMoveSpd;
				curCamEye += cross(up, camMove) * followMoveSpd;

			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//Right
				curCamCenter -= cross(up, camMove) * followMoveSpd;
				curCamEye -= cross(up, camMove) * followMoveSpd;

			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				curCamCenter = curCamCenter - (followMoveSpd * camMove);
				//Backward
				curCamEye = curCamEye - (camMove * followMoveSpd);

				if (curCamEye.y <= 0)
				{
					curCamEye.y = 0;
				}

			}
		}
		else if (!isOverheadView && (possessedActor)) // When possessing an actor the input keys will update its position
		{
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				possessedActor->position += (camMove * followMoveSpd);
			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				possessedActor->position -= (camMove * followMoveSpd);
			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				possessedActor->position += cross(up, camMove) * followMoveSpd;
			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				possessedActor->position -= cross(up, camMove) * followMoveSpd;
			}
		}
		else if (!camUpdate && isOverheadView) // --- If the camera is in overhead view
		{
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//curCamCenter.x -= (camMove.x * overheadMoveSpd);
				//curCamCenter.z -= (camMove.z * overheadMoveSpd);
				oCamEye.x += (camMove.x * overheadMoveSpd);
				oCamEye.z += (camMove.z * overheadMoveSpd);

				/*if (oCamEye.y <= 0)
				{
					oCamEye.y = 0;
				}*/

			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//Left
				//curCamCenter -= cross(up, camMove) * overheadMoveSpd;
				oCamEye += cross(up, camMove) * overheadMoveSpd;

			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//Right
				//curCamCenter += cross(up, camMove) * overheadMoveSpd;
				oCamEye -= cross(up, camMove) * overheadMoveSpd;

			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				// Backwards
				//curCamCenter.x += (camMove.x * overheadMoveSpd);
				//curCamCenter.z += (camMove.z * overheadMoveSpd);
				oCamEye.x -= (camMove.x * overheadMoveSpd);
				oCamEye.z -= (camMove.z * overheadMoveSpd);

				/*if (curCamEye.y <= 0)
				{
					curCamEye.y = 0;
				}
*/
			}
		}

		//--- Keys that act the same regardless of the camera's view
		if (key == GLFW_KEY_V && action == GLFW_PRESS) // Change Camera View
		{
			if (isOverheadView)
			{
				camUpdate = true;
			}
			else
			{
				isOverheadView = true;
			}
			
			// isOverheadView = !isOverheadView;
			/*if (!isOverheadView)
			{
				isOverheadView = true;
			}*/
			
		}
		else if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			isCaptureCursor = !isCaptureCursor;
			printf("Cursor Capture is %d\n", isCaptureCursor);
			int width, height;
			glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
			glfwSetCursorPos(window, width / 2.0f, height / 2.0f);
			
		}
		else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		{
			printf("curCamCenter is: x:%f y:%f z:%f\n", curCamCenter.x, curCamCenter.y, curCamCenter.z);
			printf("curCamEye eye is: x:%f y:%f z:%f\n", curCamEye.x, curCamEye.y, curCamEye.z);
			printf("cur rot XYZ is: x:%f y:%f z:%f\n", x, y, z);
			printf("cur isOverhead is :%d\n", isOverheadView);

		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		return;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS) // Attempt at ray casting done here
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;

			int width, height;
			glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

			float normX = (2.0f * posX) / width - 1.0f;
			float normY = 1.0f - (2.0f * posY) / height;
			float normZ = 1.0f;
			//Normalized device coordinates of mouse click
			vec3 ray_nds = vec3(normX, normY, normZ);
			//make z point forward (not 100% sure why this isnt done above)
			vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

			//projection matrix
			float aspect = width / (float)height;
			mat4 ourProjection = perspective(45.0f, aspect, 0.01f, 100.0f);
			//go backwards in pipeline from clip space to eye space(?) (only for x,y)
			vec4 ray_eye = inverse(ourProjection) * ray_clip;
			ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
			//View matrix
			mat4 ourView = lookAt(curCamEye, curCamCenter, up);
			vec4 ray_wor_temp = inverse(ourView) * ray_eye;
			vec3 ray_wor = vec3(ray_wor_temp.x, ray_wor_temp.y, ray_wor_temp.z);
			ray_wor = normalize(ray_wor); //Ray direction vector normalized
			//printf("ray in world coordinates: %f, %f, %f\n", ray_wor.x, ray_wor.y, ray_wor.z);


			if(whoseTurn == 1) {
                //Perform team 1 ray trace operations
                TeamOneRayTrace(ray_wor);
            }
			else{
			    TeamTwoRayTrace(ray_wor);
			}

			// Go back to the overhead view after shooting
			if (!isOverheadView)
			{
				isOverheadView = true;


				//Switch whose turn it is
				if(whoseTurn == 1){
				    whoseTurn = 2;
				}
				else{
				    whoseTurn = 1;
				}
			}
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

		// --- Change camera movement based on different cursor modes (Select mode vs camera mode)
		if (isCaptureCursor) {

			//Set input mode
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			// Update pitch and yaw
			phi = phi + (-newY / 400);
			theta = theta + (newX / 400);

			// Limit the angle of V up and down to 80 deg
			if (phi > 80 * to_radians)
			{
				phi = 80 * to_radians; // restrict to 80 degrees
			}
			else if (phi < -80 * to_radians)
			{
				phi = -(80 * to_radians);
			}
		}
		else // Don't rotate the camera when the cursor is visible
		{ 
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		

		

		//Update the coordinates of the temps
		tempY = tempY + newY;
		tempX = tempX + newX;

	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	void TeamOneRayTrace(vec3 ray_wor){
        for (int i = 0; i < robotUnits.size(); i++) {


            GameObject currObject = *robotUnits[i];

            bool isClicked = RayTraceCamera(ray_wor, robotUnits[i]);

            if (isClicked && possessedActor == NULL && isOverheadView) {

                robotUnits[i]->isPosessed = true;
                possessedActor = robotUnits[i]; // tell the interpolate function that it should possess the clicked object

            }
            // }

        }

        for(int i = 0; i < alienUnits.size(); i++){

            GameObject currObject = *alienUnits[i];

            bool isClicked = RayTraceCamera(ray_wor, alienUnits[i]);

            if(isClicked && possessedActor != NULL && !isOverheadView){

                printf("Hit Object: %s\n", currObject.nameObj.c_str()); // c_str() is used to make the c++ string a c string
                alienUnits[i]->beenShot = true; // Indicate the actor has been 'shot' TEMP SOLUTION
            }
        }
	}

	void TeamTwoRayTrace(vec3 ray_wor){
        for (int i = 0; i < alienUnits.size(); i++) {


            GameObject currObject = *alienUnits[i];

            bool isClicked = RayTraceCamera(ray_wor, alienUnits[i]);

            if (isClicked && possessedActor == NULL && isOverheadView) {

                alienUnits[i]->isPosessed = true;
                possessedActor = alienUnits[i]; // tell the interpolate function that it should possess the clicked object

            }
            // }

        }

        for(int i = 0; i < robotUnits.size(); i++){

            GameObject currObject = *robotUnits[i];

            bool isClicked = RayTraceCamera(ray_wor, robotUnits[i]);

            if(isClicked && possessedActor != NULL && !isOverheadView){

                printf("Hit Object: %s\n", currObject.nameObj.c_str()); // c_str() is used to make the c++ string a c string
                robotUnits[i]->beenShot = true; // Indicate the actor has been 'shot' TEMP SOLUTION
            }
        }
	}



	bool RayTraceCamera(vec3 rayDir, shared_ptr<GameObject> currObjectPointer)
	{
		vec3 dirfrac = vec3(1.0f / rayDir.x, 1.0f / rayDir.y, 1.0f / rayDir.z);

		GameObject currObject = *currObjectPointer;

		vec3 lb = currObject.bboxCenter - (currObject.bboxSize / 2.0f);
		vec3 rt = currObject.bboxCenter + (currObject.bboxSize / 2.0f);

		float t1 = (lb.x - curCamCenter.x)*dirfrac.x;
		float t2 = (rt.x - curCamCenter.x)*dirfrac.x;
		float t3 = (lb.y - curCamCenter.y)*dirfrac.y;
		float t4 = (rt.y - curCamCenter.y)*dirfrac.y;
		float t5 = (lb.z - curCamCenter.z)*dirfrac.z;
		float t6 = (rt.z - curCamCenter.z)*dirfrac.z;

		float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
		float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

		float t;

		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;
		return true;
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
		prog->addAttribute("vertTex");
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
		bunnyShape->loadMesh(resourceDirectory + "/bman.obj");
		bunnyShape->resize();
		bunnyShape->init();
		// bman works but throws hella verteTex things

		// Initialize the bunny obj mesh VBOs etc
		maRobotShape = make_shared<Shape>();
		maRobotShape->loadMesh(resourceDirectory + "/bunny.obj"); // has vertTexure issues
		maRobotShape->resize();
		maRobotShape->init();

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
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		float velocity = 0.0f;
		glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		groundbox = make_shared<GameObject>("groundbox", cube, resourceDirectory, prog, position, velocity, orientation, false, 0);


		// Setup temp player bunny
		position = vec3(0.0f, 0.0f, -25.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> PlayerBun = make_shared<GameObject>("player", bunnyShape, "../resources/", prog, position, 0, orientation, true, 1);
		sceneActorGameObjs.push_back(PlayerBun);
		robotUnits.push_back(PlayerBun);

		// Setup the second robot
		position = vec3(20.0f, 0.0f, -25.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> NPCBun = make_shared<GameObject>("robot2", bunnyShape, "../resources/", prog, position, 0, orientation, true, 1);
		sceneActorGameObjs.push_back(NPCBun);
		robotUnits.push_back(NPCBun);

		// Setup the third robot
		position = vec3(-20.0f, 0.0f, -25.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> robot3 = make_shared<GameObject>("robot3", bunnyShape, "../resources/", prog, position, 0, orientation, true, 1);
		sceneActorGameObjs.push_back(robot3);
		robotUnits.push_back(robot3);
		
		// Setup the forth robot
		position = vec3(30.0f, 0.0f, -25.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> robot4 = make_shared<GameObject>("robot4", bunnyShape, "../resources/", prog, position, 0, orientation, true, 1);
		sceneActorGameObjs.push_back(robot4);
		robotUnits.push_back(robot4);

		// Setup the first alien
		position = vec3(20.0f, 0.0f, 25.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien0 = make_shared<GameObject>("alien0", maRobotShape, "../resources/", prog, position, 0, orientation, true, 2);
		sceneActorGameObjs.push_back(alien0);
		alienUnits.push_back(alien0);
		

		// Setup the 2nd alien
		position = vec3(10.0f, 0.0f, 20.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien1 = make_shared<GameObject>("alien1", maRobotShape, "../resources/", prog, position, 0, orientation, true, 2);
		sceneActorGameObjs.push_back(alien1);
		alienUnits.push_back(alien1);

		// Setup the 3rd alien
		position = vec3(10.0f, 0.0f, 15.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien2 = make_shared<GameObject>("alien2", maRobotShape, "../resources/", prog, position, 0, orientation, true, 2);
		sceneActorGameObjs.push_back(alien2);
		alienUnits.push_back(alien2);

		// Setup the 4th alien
		position = vec3(15.0f, 0.0f, 10.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien3 = make_shared<GameObject>("alien3", maRobotShape, "../resources/", prog, position, 0, orientation, true, 2);
		sceneActorGameObjs.push_back(alien3);
		alienUnits.push_back(alien3);

		// ---TEMPORARY CUBE TERRAIN -- 
		/*position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = 0.0f;
		orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		shared_ptr<GameObject> terrainOne = make_shared<GameObject>("terrain1", cube, resourceDirectory, prog, position, velocity, orientation, false, 0);
		sceneActorGameObjs.push_back(terrainOne);

		position = glm::vec3(2.5f, 0.0f, 0.0f);
		velocity = 0.0f;
		orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		shared_ptr<GameObject> terrainTwo = make_shared<GameObject>("terrain2", cube, resourceDirectory, prog, position, velocity, orientation, false, 0);
		sceneActorGameObjs.push_back(terrainTwo);*/

		for (int i = 0; i < 20; i++) {
			float randX = -40.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (40.0f - -40.0f)));
			float randZ = -60.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (60.0f - -60.0f)));


			position = glm::vec3(randX, 0.0f, randZ);
			velocity = 0.0f;
			orientation = glm::vec3(0.0f, 0.0f, 0.0f);
			shared_ptr<GameObject> terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, prog, position, velocity, orientation, false, 0);
			sceneActorGameObjs.push_back(terrainTemp);

		}
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


	void renderPlayerBbox(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, bool overheadView)
	{
		prog->bind();
		// Recompute the position of the box b4 drawing it

		/*if (overheadView)
		{
			p1_bboxCenter = oCamCenter;
		}
		else
		{
			p1_bboxCenter = pCamCenter;
		}
		*/
		p1_bboxCenter = curCamCenter;
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

	void renderGroundPlane(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, bool overheadView)
	{
		//vec3 camLoc;
		//camLoc = curCamEye;

		prog->bind();

		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(0.f, -1.f, 0.f)); //move the plane down a little bit in y space 
		M->scale(vec3(40.f, .1f, 120.f)); // turn the cube into a plane

		groundbox->step(deltaTime, M, P, curCamEye, curCamCenter, up);
		//add uniforms to shader
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));

		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(curCamEye, curCamCenter, up)));
		glUniform3f(prog->getUniform("lightSource"), 0, 88, 10);
		//glUniform3f(prog->getUniform("pCamEye"), 0, 10, 0);
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

	

	void renderSceneActors(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, bool overheadView, int offsetX, int offsetZ)
	{
		//vec3 camLoc = curCamEye;
		//if (overheadView)
		//{
		//	camLoc = oCamEye; // Change to use oCampCamEye

		//}
		//else // Possession follow cam
		//{
		//	camLoc = pCamEye;
		//}

		
		prog->bind(); // Bind the Simple Shader


		// 
		for (int i = 0; i < sceneActorGameObjs.size(); i++) {


			if(sceneActorGameObjs[i]->isRender == true){
				M->pushMatrix();
				M->loadIdentity();

				// Update the position of the rabbit based on velocity, time elapsed also updates the center of the bbox
				
				sceneActorGameObjs[i]->step(deltaTime, M, P, curCamEye, curCamCenter, up);
				// bunBun->DoCollisions()


				//add uniforms to shader
				// Set the materials of the bunny depending on if the player has hit it or not
				if (sceneActorGameObjs[i]->beenShot)
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

				glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(curCamEye, curCamCenter, up)));
				
				glUniform3f(prog->getUniform("lightSource"), 0, 88, 10);
				sceneActorGameObjs[i]->DrawGameObj(); // Draw the bunny model and render bbox

				M->popMatrix();
			}
			
		}


		prog->unbind(); // Unbind the Simple Shader

		return;

	}

	void interpolateCamera(float interp)
	{
		// Check if interp is too large
		if (interp > 1.0f)
		{
			interp = 1.0f;
		}
		// 0.0 is starting position and 1.0 is final position
		if (isOverheadView) // Interpolate from the overhead view to the possessed game object
		{
			if (possessedActor != NULL)
			{
				pCamEye = possessedActor->position;
				if (interp > 0.55f) // Turn of the rendering of the possessed object
				{
					possessedActor->isRender = false;
				}
			}
			//for (int i = 0; i < sceneActorGameObjs.size(); i++)
			//{
			//	if ((sceneActorGameObjs[i])->isPosessed == true)
			//	{
			//		printf("Using a possessed object's location");
			//		GameObject posActor = *sceneActorGameObjs[i];
			//		pCamEye = posActor.position; // use the position of the game object with an offset
			//		if (interp > 0.9f) // Turn of the rendering of the possessed object
			//		{
			//			sceneActorGameObjs[i]->isRender = false;
			//		}
			//		break;
			//	}
			//}

			// Compute 
			float newx = ((1.0f - interp) * curCamEye.x) + (interp * pCamEye.x);
			float newy = ((1.0f - interp) * curCamEye.y) + (interp * pCamEye.y);
			float newz = ((1.0f - interp) * curCamEye.z) + (interp * pCamEye.z);
			curCamEye = vec3(newx, newy, newz);

			newx = ((1.0f - interp) * (curCamEye.x + ox)) + (interp * (pCamEye.x + (radius * cos(phi)*cos(theta) )));
			newy = ((1.0f - interp) * (curCamEye.y + oy)) + (interp * (pCamEye.y + (radius * sin(phi))));
			newz = ((1.0f - interp) * (curCamEye.z + oz)) + (interp * (pCamEye.z + (radius * cos(phi)*sin(theta))));
			curCamCenter = vec3(newx, newy, newz);

			camMove = vec3( ((1.0f - interp) * ox ) + (interp * x),
				((1.0f - interp) * oy ) + (interp * y),
				((1.0f - interp) * oz ) + (interp * z));
		}
		else
		{
			// Compute an interpolated camera
			/*float newx = ((1.0f - interp) * curCamEye.x) + (interp * oCamEye.x);
			float newy = ((1.0f - interp) * curCamEye.y) + (interp * oCamEye.y);
			float newz = ((1.0f - interp) * curCamEye.z) + (interp * oCamEye.z);
			curCamEye = vec3(newx, newy, newz);

			newx = ((1.0f - interp) * (curCamEye.x + (pCamEye.x + (radius * cos(phi)*cos(theta))))) + (interp * (oCamEye.x + ox));
			newy = ((1.0f - interp) * (curCamEye.y + (radius * sin(phi)))) + (interp * (oCamEye.y + oy));
			newz = ((1.0f - interp) * (curCamEye.z + (pCamEye.z + (radius * cos(phi)*sin(theta))))) + (interp * (oCamEye.z + oz));
			curCamCenter = vec3(newx, newy, newz);

			camMove = vec3(((1.0f - interp) * x) + (interp * ox),
				((1.0f - interp) * y ) + (interp * oy),
				((1.0f - interp) * z ) + (interp * oz));*/

			// Immediately swap camera position
			
		}
	}

	void updateGameLogic()
	{
		// printf("Update Game Logic\n");
		int counter = 0;
		for (int i = 0; i < alienUnits.size(); i++)
		{
			if (alienUnits[i]->beenShot)
			{
				counter++;
			}
		}
		if (counter == alienUnits.size())
		{
			
			printf("THE ROBOTS HAVE WON!\n");

		}
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
		// ---- At intervals print out information to the console
		if (currentTime - lastTime >= 1.0 ) {
			//printf("MS/FPS: %f    FPS: %f\n", 1000.0 / double(nbFrames) , double(nbFrames)); // Print out ms/fps and frame rate
			//printf("Objects on the ground: %d\n", sceneActorGameObjs.size()); // Print out the number of objects in the game
			//printf("Objects Collided with %d\n\n", p1Collisions);
			nbFrames = 0;
			lastTime += 1.0;
		}

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		elapsedTime += deltaTime;


		//--TODO: Move maybe. Spawns random bunnies
		//bunSpawn -= deltaTime;
		//if (bunSpawn <= 0 && (sceneActorGameObjs.size() < 40)) // Spawn a bunny if the timer is up and there are less than 25 in the scene
		//{
		//	printf("spawn bunny\n");
		//	bunSpawn = bunSpawnReset;
		//	//Template for random float
		//	//float r3 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
		//	float randX = -30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f - -30.0f)));
		//	float randZ = -30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f - -30.0f)));
		//	float randVel = 1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (8.0f - 1.0f)));
		//	
		//	string BunName = "bunBun" + to_string(bunbunCounter);

		//	vec3 position = vec3(randX, 0.0f, randZ);

		//	vec3 orientation = vec3(0.0f, 0.0f, 1.0f);

		//	shared_ptr<GameObject> newBunny = make_shared<GameObject>(BunName, bunnyShape, "../resources/", prog, position, randVel, orientation, true);
		//	sceneActorGameObjs.push_back(newBunny);

		//}


		


		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width / (float)height;



		// Setup yaw and pitch of camera for lookAt()
		if (!isOverheadView) // Possession
		{
			if (possessedActor)
			{
				curCamEye = possessedActor->position;
			}
			x = radius * cos(phi)*cos(theta);
			y = radius * sin(phi);
			z =  radius * cos(phi)*sin(theta);
			//printf("rots phi: %d, theta: %d \n", phi, theta);
			curCamCenter = curCamEye + vec3(x, y, z);
			camMove = vec3(x, y, z);
		}
		else // Overhead
		{
			// Hard code the look direction for the over view camera
			ox = 0.45f;
			oy = -1.0f;
			oz = -0.00;
			vec3 lookDir = vec3(ox, oy, oz);
			
			curCamCenter = curCamEye + lookDir;
			camMove = vec3(ox, oy, oz);
		}
		//oCamCenter = oCamEye; // Trying


		//for(int i = 0; i < sceneActorGameObjs.size(); )


		// Check if the camera should be interpolated
		static float camInterp = 0.0f;
		if (camInterp > 1.0f && camUpdate)
		{
			camInterp = 0.0f;
			camUpdate = false;

			
			isOverheadView = false; // Toggle the currentCamera after interpolation is finished
		}

		if (camInterp <= 1.0f && camUpdate) // interpolate from the overhead camera to the possesd gameobjects view
		{
			camInterp += 0.01f;
			interpolateCamera(camInterp);
		}
		else if (isOverheadView) // Snap from the players camera to the ovehead view
		{
			curCamEye = oCamEye;

			float onewx = oCamEye.x + ox;
			float onewy = oCamEye.y + oy;
			float onewz = oCamEye.z + oz;
			curCamCenter = vec3(onewx, onewy, onewz);

			if ((possessedActor != NULL) && (possessedActor->isRender == false)) // Turn on rendering for the possessed actor when going back
			{
				possessedActor->isRender = true;
				possessedActor = NULL; // Remove the possessed actor
			}
		}
		//else if (!isOverheadView)
		//{
		//	x = radius * cos(phi)*cos(theta);
		//	y = radius * sin(phi);
		//	z = radius * cos(phi)*sin(theta);
		//	//printf("rots phi: %d, theta: %d \n", phi, theta);
		//	curCamCenter = curCamEye + vec3(x, y, z);
		//	camMove = vec3(x, y, z);
		//}
		/*if (!camUpdate && !isOverheadView)
		{
			isOverheadView = true;
		}*/

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		
		auto M = make_shared<MatrixStack>();


		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f); // First arguement is Camera FOV, only 45 and 90 seem to be working well


		/*Draw the actual scene*/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		M->pushMatrix(); // Matrix for the Scene
		renderGroundPlane(M, P, isOverheadView); //draw the ground plane

		// Update the position of the players bbox
		renderPlayerBbox(M, P, isOverheadView);

		//renderAnimSphere(M, P, 0, 0, 0); //draw the hierarchical modeled animated spheres

		M->pushMatrix();
		//checkAllGameObjects();
		renderSceneActors(M, P, isOverheadView, 0, 0);
		checkAllGameObjects();
		//bunBun->DoCollisions(groundbox);
		M->popMatrix();


		M->popMatrix(); // Pop Scene Matrix

	}

};


int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources/";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1080); //was 512,512.
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);


	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Update the games logic
		application->updateGameLogic();

		// Render scene.
		application->render();


		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());

		// Poll for and process events.
		glfwPollEvents();


	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
