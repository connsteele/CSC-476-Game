#include <iostream>
#include <algorithm>
#include <ctime>
#define _USE_MATH_DEFINES //use to access M_PI
#include "math.h"
#include "glad/glad.h"

//#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h" // MIGHT HAVE TO REMOVE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "BaseCode/GLSL.h"
#include "BaseCode/Program.h"
#include "BaseCode/Shape.h"
#include "BaseCode/WindowManager.h"
//#include "BaseCode/GLTextureWriter.h"
#include "BaseCode/MatrixStack.h"
// #include "BaseCode/Texture.h"
#include "GameObject.h" // Our Game Object Class
#include "ourCoreFuncs.h"
#include "UIController.h"
#include "Weapon.h"


#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

//------ Globals
vector<tinyobj::shape_t> robotDefault, robot1;
vector<tinyobj::material_t> robotDefaultMat, robot1Mat;
GLuint buffRobotDefault, buffRobotNormDefault, buffRobot1, buffRobot1Norm;

#define WINDOWSIZE_X 1920
#define WINDOWSIZE_Y 1080

//---
int p1Collisions = 0;

// -- Variables for possessed actor
shared_ptr<GameObject> possessedActor = NULL;

//--- Variables for players bbox 
GLuint p1_vbo_vertices;
GLuint p1_ibo_elements;
vec3 p1_bboxSize, p1_bboxCenter;
mat4 p1_bboxTransform;

// --- Variables to store textures into
GLuint Tex_Floor, Tex_Wall, Tex_Hex, Tex_Fan;
std::shared_ptr<Program> progTerrain;

// --- Variables for Geometry
GLuint bufCubeNormal, bufCubeTexture, bufCubeIndex;

//--- Vector of all actor game objects plus arrays of player units and enemy units
vector<shared_ptr<GameObject> > sceneActorGameObjs, sceneTerrainObjs, AllGameObjects, usedRobotUnits, usedAlienUnits;
vector<shared_ptr<Weapon> > weapons;
vector<shared_ptr<GameObject> > robotUnits;
vector<shared_ptr<GameObject> > alienUnits;

vector<vec3> coverCubesLocs;

//Camera Timing
float deltaTime = 0.0f, lastTime = glfwGetTime();
float lastFrame = 0.0f;
int nbFrames = 0;
float elapsedTime = 0.0f;

//Turn Time
double turnStartTime = 0;
//durration of possesion in seconds
int turnLength = 50;

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
vec3 oCamEye = vec3(-7.0f, 15.0f, -40.00);

// Current Camera
vec3 curCamEye = oCamEye;
vec3 curCamCenter;
// ------------------------

//Animation:
float orbRotate = 0.0;
float smallRotate = 0.0;
float bunnyRotate = 0.0;

//UI
UIController mainMenuUI;
UIController overViewUI;
UIController firstPersonUI;
UIController debugUI;
#define DEBUG_MODE 1   //!= 0 to display debug UI window; 0 to hide


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

	//Keep track of how many units remain for each team
	int numAlienUnits = 4;
	int numRobotUnits = 4;
	

	//Player Gravity Variables
	float acceleration = -9.8f;
	float velocity = 0.0f;
	bool canJump = true;
	bool readyToSwitch = false;
	

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;

	// Access OBJ files
	shared_ptr<Shape> bunnyShape, maRobotShape;
	shared_ptr<Shape> cube;
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> gun, shotgun;

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

    // See if player will hit object if they move
	bool ComputePlayerHitObjects(vec3 NewCenter){
	    for(int i = 0; i < AllGameObjects.size(); i++){
	        // Dont check collision with self
	        if(AllGameObjects[i] != possessedActor) {
                bool collisionX = NewCenter.x + possessedActor->bboxSize.x >= AllGameObjects[i]->bboxCenter.x &&
                                  AllGameObjects[i]->bboxCenter.x + AllGameObjects[i]->bboxSize.x >= NewCenter.x;
                bool collisionY = NewCenter.y + possessedActor->bboxSize.y >= AllGameObjects[i]->bboxCenter.y &&
                                  AllGameObjects[i]->bboxCenter.y + AllGameObjects[i]->bboxSize.y >= NewCenter.y;
                bool collisionZ = NewCenter.z + possessedActor->bboxSize.z >= AllGameObjects[i]->bboxCenter.z &&
                                  AllGameObjects[i]->bboxCenter.z + AllGameObjects[i]->bboxSize.z >= NewCenter.z;

                bool HitResult = collisionX && collisionY && collisionZ;

                if (HitResult) {
                    return HitResult;
                }
            }

	    }
        return false;
	}

	//Collect Weapon when colliding
	void checkWeaponCollection(vec3 NewCenter){
		for(int i = 0; i < weapons.size(); i++){
            bool collisionX = NewCenter.x + possessedActor->bboxSize.x >= weapons[i]->bboxCenter.x &&
                                  weapons[i]->bboxCenter.x + weapons[i]->bboxSize.x >= NewCenter.x;
            bool collisionY = NewCenter.y + possessedActor->bboxSize.y >= weapons[i]->bboxCenter.y &&
                                  weapons[i]->bboxCenter.y + weapons[i]->bboxSize.y >= NewCenter.y;
            bool collisionZ = NewCenter.z + possessedActor->bboxSize.z >= weapons[i]->bboxCenter.z &&
                                  weapons[i]->bboxCenter.z + weapons[i]->bboxSize.z >= NewCenter.z;

            bool HitResult = collisionX && collisionY && collisionZ;

            if (HitResult) {
				int weaponType = weapons[i]->weaponType;
				weapons.erase(weapons.begin() + i);
				possessedActor->currWeapon = weaponType;
            	return;
            }

	    }
        return;
	}


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{

		float followMoveSpd = 14.0f * deltaTime;
		float overheadMoveSpd = 110.0f * deltaTime;

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
		    //float FixedHeight = 0.1f; // Used to lock vertical position of models
		    // possessedActor->position.y = 0.1f;

			float CurrentYPosition = possessedActor->position.y;

			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
			    vec3 newPosition = possessedActor-> position + (camMove * followMoveSpd);
				newPosition.y = CurrentYPosition;//+ 0.1f;
			    bool hitObject = ComputePlayerHitObjects(newPosition);
			    if(!hitObject) {
                    possessedActor->position = newPosition;
					checkWeaponCollection(newPosition);
                }
			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
			    vec3 newPosition = possessedActor->position - (camMove * followMoveSpd);
			    newPosition.y = CurrentYPosition;// + 0.1f;
                bool hitObject = ComputePlayerHitObjects(newPosition);
                if(!hitObject) {
                    possessedActor->position = newPosition;
					checkWeaponCollection(newPosition);
                }
			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
			    vec3 newPosition = possessedActor->position + cross(up, camMove) * followMoveSpd;
			    newPosition.y = CurrentYPosition;// + 0.1f;
                bool hitObject = ComputePlayerHitObjects(newPosition);
                if(!hitObject) {
                    possessedActor->position = newPosition;
					checkWeaponCollection(newPosition);
                }
			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
			    vec3 newPosition = possessedActor->position - cross(up, camMove) * followMoveSpd;
			    newPosition.y = CurrentYPosition;// + 0.1f;
                bool hitObject = ComputePlayerHitObjects(newPosition);
                if(!hitObject) {

                    possessedActor->position = newPosition;
					checkWeaponCollection(newPosition);
                }
			}
			else if(key == GLFW_KEY_SPACE && (action == GLFW_PRESS) && canJump){
			    velocity = 7.0f;
			    canJump = false;
			}
		}
		else if (!camUpdate && isOverheadView) // --- If the camera is in overhead view
		{
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				// Forward
				oCamEye.x += (camMove.x * overheadMoveSpd);
				oCamEye.z += (camMove.z * overheadMoveSpd);

			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				// Left
				oCamEye += cross(up, camMove) * overheadMoveSpd;

			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				//Right
				oCamEye -= cross(up, camMove) * overheadMoveSpd;

			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				// Backwards
				oCamEye.x -= (camMove.x * overheadMoveSpd);
				oCamEye.z -= (camMove.z * overheadMoveSpd);
			}
		}

		//--- Keys that act the same regardless of the camera's view
		if (key == GLFW_KEY_V && action == GLFW_PRESS) // Change Camera View
		{
			if (isOverheadView && (possessedActor != NULL))
			{
				camUpdate = true;
			}
			else
			{
				isOverheadView = true;
				firstPersonUI.setRender(false);
				overViewUI.setRender(true);
			}			
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

            vec3 ray_wor = GenerateRay(posX, posY); // Generate ray from mouse click


			if(whoseTurn == 1) {
                //Perform team 1 ray trace operations
                TeamOneRayTrace(ray_wor);
            }
			else{
				//Perform team 2 ray trace operations
			    TeamTwoRayTrace(ray_wor);
			}

			// Go back to the overhead view after shooting
			if (!isOverheadView)
			{
				//switchTurn();
				readyToSwitch = true;
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

				// Check if unit has been used (check if not empty first)
				if(!usedRobotUnits.empty()){
					if(find(usedRobotUnits.begin(), usedRobotUnits.end(), robotUnits[i]) == usedRobotUnits.end()){
						robotUnits[i]->isPosessed = true;
                		possessedActor = robotUnits[i]; // tell the interpolate function that it should possess the clicked object
						usedRobotUnits.push_back(robotUnits[i]);
						robotUnits[i]->isUsed = true;
					}
				}
				// Add first unit to array
				else{
					robotUnits[i]->isPosessed = true;
                	possessedActor = robotUnits[i]; // tell the interpolate function that it should possess the clicked object
					usedRobotUnits.push_back(robotUnits[i]);
					robotUnits[i]->isUsed = true;
				}

            }

        }

        //Only run weapon loop of possessed actor exists
        if(possessedActor != NULL){

            vector<shared_ptr<GameObject> > HitObjects;

            //if weapon is shotgun
            if(possessedActor->currWeapon == 1){

				int width, height;
				glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
				float posX = width / 2.0f; //Magic Number
				float posY = height / 2.0f; //Magic Number

                //Generate rays one time, not once for each actor
				vec3 ray_center = GenerateRay(posX, posY);
                vec3 ray_left = GenerateRay(posX - 50.0, posY);
                vec3 ray_right = GenerateRay(posX + 50.0, posY);
                vec3 ray_down = GenerateRay(posX, posY + 50.0);
                vec3 ray_up = GenerateRay(posX, posY - 50.0);

                //Check ray collisions with all game objects
                for(int i = 0; i < AllGameObjects.size(); i++){
                    bool isClicked = possessedActor->FireShotgun(ray_center, ray_left, ray_right, ray_down, ray_up, AllGameObjects[i], curCamCenter);

                    if(isClicked){
                        //If ray hit object add to vector of hit objects
                        HitObjects.push_back(AllGameObjects[i]);
                    }
                }

            }
            else if(possessedActor->currWeapon == 0){

				int width, height;
				glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
				float posX = width / 2.0f; //Magic Number
				float posY = height / 2.0f; //Magic Number

				vec3 ray_center = GenerateRay(posX, posY);

                for(int i = 0; i < AllGameObjects.size(); i++){
                    bool isClicked = possessedActor->FirePistol(ray_center, AllGameObjects[i], curCamCenter);

                    if(isClicked){
                        //If ray hit object add to vector of hit objects
                        HitObjects.push_back(AllGameObjects[i]);
                    }
                }

            }

            //Loop to see what the closest object hit was
            float minDistance = 1000000.0f;
            float minDistanceIndex = -1;

            for(int i = 0; i < HitObjects.size(); i++){
                float currDistance = distance(curCamCenter, HitObjects[i]->bboxCenter);
                if(currDistance < minDistance){
                    minDistance = currDistance;
                    minDistanceIndex = i;
                }
            }
            if(HitObjects.size() != 0) {

                if (HitObjects[minDistanceIndex]->team == 2 && !isOverheadView) {
                    //HitObjects[minDistanceIndex]->beenShot = true; // Indicate the actor has been 'shot' TEMP SOLUTION
					HitObjects[minDistanceIndex]->health -= 1.0f;
					if(HitObjects[minDistanceIndex]->health <= 0.0f){
						HitObjects[minDistanceIndex]->beenShot = true;
						numAlienUnits--;
					}
                }
            }

        }
	}

	void TeamTwoRayTrace(vec3 ray_wor){
        for (int i = 0; i < alienUnits.size(); i++) {


            GameObject currObject = *alienUnits[i];

            bool isClicked = RayTraceCamera(ray_wor, alienUnits[i]);

            if (isClicked && possessedActor == NULL && isOverheadView) {

				// check if clicked object is already in array (check if empty first)
                if(!usedAlienUnits.empty()){
					if(find(usedAlienUnits.begin(), usedAlienUnits.end(), alienUnits[i]) == usedAlienUnits.end()){
						alienUnits[i]->isPosessed = true;
                		possessedActor = alienUnits[i]; // tell the interpolate function that it should possess the clicked object
						usedAlienUnits.push_back(alienUnits[i]);
						alienUnits[i]->isUsed = true;
					}
					else
					{
						printf("Actor was already used, pick a different teammate");
					}
				}
				// Add first unit to array
				else{
					alienUnits[i]->isPosessed = true;
                	possessedActor = alienUnits[i]; // tell the interpolate function that it should possess the clicked object
					usedAlienUnits.push_back(alienUnits[i]);
					alienUnits[i]->isUsed = true;
				}

            }

        }

        //Only run weapon loop of possessed actor exists
        if(possessedActor != NULL){

            vector<shared_ptr<GameObject> > HitObjects;

            //if weapon is shotgun
            if(possessedActor->currWeapon == 1){
				int width, height;
				glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
				float posX = width / 2.0f; //Magic Number
				float posY = height / 2.0f; //Magic Number

                //Generate rays one time, not once for each actor
				vec3 ray_center = GenerateRay(posX, posY);
                vec3 ray_left = GenerateRay(posX - 50.0, posY);
                vec3 ray_right = GenerateRay(posX + 50.0, posY);
                vec3 ray_down = GenerateRay(posX, posY + 50.0);
                vec3 ray_up = GenerateRay(posX, posY - 50.0);

                //Check ray collisions with all game objects
                for(int i = 0; i < AllGameObjects.size(); i++){
                    bool isClicked = possessedActor->FireShotgun(ray_center, ray_left, ray_right, ray_down, ray_up, AllGameObjects[i], curCamCenter);

                    if(isClicked){
                        //If ray hit object add to vector of hit objects
                        HitObjects.push_back(AllGameObjects[i]);
                    }
                }

            }
            else if(possessedActor->currWeapon == 0){
				int width, height;
				glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
				float posX = width / 2.0f; //Magic Number
				float posY = height / 2.0f; //Magic Number

				vec3 ray_center = GenerateRay(posX, posY);
                for(int i = 0; i < AllGameObjects.size(); i++){
                    bool isClicked = possessedActor->FirePistol(ray_center, AllGameObjects[i], curCamCenter);

                    if(isClicked){
                        //If ray hit object add to vector of hit objects
                        HitObjects.push_back(AllGameObjects[i]);
                    }
                }

            }

            //Loop to see what the closest object hit was
            float minDistance = 1000000.0f;
            float minDistanceIndex = -1;

            for(int i = 0; i < HitObjects.size(); i++){
                float currDistance = distance(curCamCenter, HitObjects[i]->bboxCenter);
                if(currDistance < minDistance){
                    minDistance = currDistance;
                    minDistanceIndex = i;
                }
            }
            if(HitObjects.size() != 0) {

                if (HitObjects[minDistanceIndex]->team == 1 && !isOverheadView) {
                    HitObjects[minDistanceIndex]->health -= 1.0f;
					if(HitObjects[minDistanceIndex]->health <= 0.0f){
						HitObjects[minDistanceIndex]->beenShot = true;
						numRobotUnits--;
					}
                }
            }

        }
	}

	vec3 GenerateRay(double posX, double posY){
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

        return ray_wor;
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

		float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
		float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

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
		case 3: //Mine: red
			glUniform3f(prog->getUniform("MatAmb"), 0.15, 0.17, 0.12);
			glUniform3f(prog->getUniform("MatDif"), 0.83, 0.2, 0.2);
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.22, 0.22);
			glUniform1f(prog->getUniform("shine"), 20.0);
			break;
		case 4: //Mine: green
			glUniform3f(prog->getUniform("MatAmb"), 0.15, 0.17, 0.12);
			glUniform3f(prog->getUniform("MatDif"), 0.13, 0.8, 0.2);
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.22, 0.22);
			glUniform1f(prog->getUniform("shine"), 20.0);
			break;
		case 5: // White
			glUniform3f(prog->getUniform("MatAmb"), 1.0, 1.0, 1.0);
			glUniform3f(prog->getUniform("MatDif"), 1.0, 1.0, 1.0);
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
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//----- Setup Shaders -----
		// Setup the default shader program
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
		prog->addUniform("eye");
		prog->addUniform("hit"); //Uniform for determining color based on hit or not

		// Setup a terrain shader program
		progTerrain = make_shared<Program>();
		progTerrain->setVerbose(true);
		progTerrain->setShaderNames(
			resourceDirectory + "/terrain_vert.glsl",
			resourceDirectory + "/terrain_frag.glsl");
		if (!progTerrain->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progTerrain->addUniform("P");
		progTerrain->addUniform("M");
		progTerrain->addUniform("V");
		progTerrain->addUniform("eye");
		progTerrain->addAttribute("vertPos");
		progTerrain->addAttribute("vertNor");
		progTerrain->addAttribute("vertTex");
		progTerrain->addUniform("lightSource"); //lighting uniform

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

	void initOverViewUI(GLFWwindow *window) {
		overViewUI.setName("Overview");
		overViewUI.setRender(true);
		overViewUI.setSize(WINDOWSIZE_X, WINDOWSIZE_Y / 8);
		//declare UI elements
		//UIButton* testButton1 = new UIButton(vec3(0, 0, 0), vec3(255, 0, 0), 1, "testButton1");
		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//add them to UI controller
		//overViewUI.addElement(testButton1);
	}

	void initFirstPersonUI(GLFWwindow *window) {
		firstPersonUI.setName("First Person");
		firstPersonUI.setRender(false);
		firstPersonUI.setSize(WINDOWSIZE_X, WINDOWSIZE_Y / 8);
		//declare UI elements
		//UIButton* testButton2 = new UIButton(vec3(0, 0, 0), vec3(0, 255, 0), 1, "testButton2");
		UIBar* healthBar = new UIBar(vec3(0, 0, 0), vec3(0, 255, 0), 1, 5.f, "health", 0.f, 5.f);

		//add them to UI controller
		//firstPersonUI.addElement(testButton2);
		firstPersonUI.addElement(healthBar);
	}

	void initMainMenuUI(GLFWwindow *window) {

	}

	void initUI(GLFWwindow *window) {
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui::StyleColorsDark();
		ImGui::GetStyle().Alpha = 1.f;
		//ImGui::GetStyle().Colors[]
		ImGui_ImplOpenGL3_Init("#version 130");
		initOverViewUI(window);
		initFirstPersonUI(window);
		initMainMenuUI(window);
	}

	void setupCoverCubeLocations()
	{
		// X is horizontal for us, and y is vertical
		coverCubesLocs.push_back(vec3(-39.f, 0.f, -59.f));
		coverCubesLocs.push_back(vec3(-38.f, 0.f, -59.f));
		coverCubesLocs.push_back(vec3(-37.f, 0.f, -59.f));
	}

	// Used with the image of the map
	int getColor(unsigned char* data, int width, int x, int y, int rgb)
	{
		// rgb should be 0 for red, 1 for green, 2 for blue
		int index = (3 * x) + (3 * width * y) + rgb;
		return (int)data[index];
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// ------ Load Models --------
		//Initialize the geometry to render a quad to the screen
		initQuad();

		// Initialize the bunny obj mesh VBOs etc
		bunnyShape = make_shared<Shape>();
		bunnyShape->loadMesh(resourceDirectory + "/bunny.obj");
		bunnyShape->resize();
		bunnyShape->init();
		// bman works but throws hella verteTex things

		// Initialize the bunny obj mesh VBOs etc
		maRobotShape = make_shared<Shape>();
		maRobotShape->loadMesh(resourceDirectory + "/robot0.obj"); // has vertTexure issues
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

		// Initialize the pistol OBJ model
		gun = make_shared<Shape>();
		gun->loadMesh(resourceDirectory + "/Pistol.obj");
		gun->resize();
		gun->init();

		// Initialize the gun OBJ model
		shotgun = make_shared<Shape>();
		shotgun->loadMesh(resourceDirectory + "/shotgun.obj");
		shotgun->resize();
		shotgun->init();

		// ----- Load the models for the idle animation
		// Robot Default pose
		string err;
		bool fbool;
		fbool = false;
		fbool = tinyobj::LoadObj(robotDefault, robotDefaultMat, err, "../resources/robot0.obj");
		if (robotDefault.size() <= 0) {
			cout << "robotDefault size < 0";
			return;
		}
		glGenBuffers(1, &buffRobotDefault);
		glBindBuffer(GL_ARRAY_BUFFER, buffRobotDefault);
		glBufferData(GL_ARRAY_BUFFER, robotDefault[0].mesh.positions.size() * sizeof(float), &robotDefault[0].mesh.positions[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // layout(location = 0)
		//Bind Default FaceNorms
		glGenBuffers(1, &buffRobotNormDefault);
		glBindBuffer(GL_ARRAY_BUFFER, buffRobotNormDefault);
		glBufferData(GL_ARRAY_BUFFER, robotDefault[0].mesh.normals.size() * sizeof(float), &robotDefault[0].mesh.normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // layout(location = 1)

		// Robots Pose 2
		fbool = false;
		fbool = tinyobj::LoadObj(robot1, robot1Mat, err, "../resources/robot0.obj");
		if (robot1.size() <= 0) {
			cout << "robot1 size < 0";
			return;
		}
		glGenBuffers(1, &buffRobot1);
		glBindBuffer(GL_ARRAY_BUFFER, buffRobot1);
		glBufferData(GL_ARRAY_BUFFER, robot1[0].mesh.positions.size() * sizeof(float), &robot1[0].mesh.positions[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // layout(location = 0)
		//Bind Default FaceNorms
		glGenBuffers(1, &buffRobot1Norm);
		glBindBuffer(GL_ARRAY_BUFFER, buffRobot1Norm);
		glBufferData(GL_ARRAY_BUFFER, robot1[0].mesh.normals.size() * sizeof(float), &robot1[0].mesh.normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // layout(location = 1)


		// Setup player bbox
		initPlayerBbox();

		// ---------- Setup Other Geometery -----------
		
		

		// ---------- Load Images-----------

		//--- Load the image of the floor
		string str = resourceDirectory + "/images/scifiFloor.bmp";
		char filepath[1000]; // Char array
		int width, height, channels;
		strcpy(filepath, str.c_str()); // copy the string into the char array
		unsigned char* dataLayout = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Tex_Floor);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Tex_Floor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip maps for smaller than native size
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // mip maps for larger than normal size
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataLayout);
		glGenerateMipmap(GL_TEXTURE_2D);


		str = resourceDirectory + "/images/scifiWall.jpg";
		strcpy(filepath, str.c_str()); // copy the string into the char array
		dataLayout = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Tex_Wall);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Tex_Wall);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip maps for smaller than native size
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // mip maps for larger than normal size
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataLayout);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/images/hexPlating.png";
		strcpy(filepath, str.c_str()); // copy the string into the char array
		dataLayout = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Tex_Hex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Tex_Hex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip maps for smaller than native size
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // mip maps for larger than normal size
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataLayout);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/images/fan.jpg";
		strcpy(filepath, str.c_str()); // copy the string into the char array
		dataLayout = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Tex_Fan);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Tex_Fan);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip maps for smaller than native size
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // mip maps for larger than normal size
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataLayout);
		glGenerateMipmap(GL_TEXTURE_2D);


		//---  Load the image of the map file
		str = resourceDirectory + "/images/Map1.bmp";
		strcpy(filepath, str.c_str()); // copy the string into the char array
		dataLayout = stbi_load(filepath, &width, &height, &channels, 3);
		// dataLayout = stbi_load(filepath, &width, &height, &channels, 3);
		//-- Map Tile Properties that never change
		glm::vec3 tileOrientation = glm::vec3(0.0f, 0.0f, 0.0f); // Current tile's orientation, Will always be vec3(0.0f)
		glm::vec3 tilePos = glm::vec3(0.0f);
		float tileScale = 2.0f;
		shared_ptr<GameObject> terrainTemp;
		float verticalOffset = (height * tileScale)/2.0f, horizontalOffset = (width * tileScale)/2.0f;
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				int red = getColor(dataLayout, width, i, j, 0);
				int green = getColor(dataLayout, width, i, j, 1);
				int blue = getColor(dataLayout, width, i, j, 2);
				printf("current Width: %d, Height: %d, Color: %d %d %d\n", i, j, red, green, blue);
				//--- Set the position of the object based on the color of the current pixel in the image
				if ((red == 255) && (green == 255) && (blue == 255)) // If the color is white draw a ground tile 
				{
					tilePos = glm::vec3(verticalOffset + j * -tileScale, -tileScale, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size()-1]->isGroundTile = true;
                    AllGameObjects.push_back(terrainTemp);
				}
				else if ((red == 255) && (green == 255) && (blue == 0)) // If the color is yellow draw a upsairs tile
				{
					tilePos = glm::vec3(verticalOffset + j * -tileScale, -tileScale/2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isUpperTile = true;
                    AllGameObjects.push_back(terrainTemp);
				}
				else if ((red == 255) && (green == 0) && (blue == 255)) // If the color is purple draw a ground cover cube
				{
					tilePos = glm::vec3(verticalOffset + j * -tileScale, 0.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isCoverTile = true;
                    AllGameObjects.push_back(terrainTemp);
				}
				else if ((red == 0) && (green == 255) && (blue == 0)) // If the color is green add a shotgun
				{
					tilePos = glm::vec3(verticalOffset + j * -tileScale, -tileScale, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isJumpTile = true;
                    AllGameObjects.push_back(terrainTemp);

					// ---- Add Weapons to the game
					vec3 position = tilePos;
					position.y += 2.0f;
					shared_ptr<Weapon> shotty = make_shared<Weapon>("shotbun", shotgun, "../resources/", prog, position, tileOrientation, true, 2, false, 1);
					weapons.push_back(shotty);

				}
				else if ((red == 0) && (green == 0) && (blue == 255)) // If the color is blue upper cover tile
				{
					tilePos = glm::vec3(verticalOffset + j * -tileScale, -tileScale/2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isUpperTile = true;

					tilePos = glm::vec3(verticalOffset + j * -tileScale, tileScale/2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isUpperCoverTile = true;
                    AllGameObjects.push_back(terrainTemp);
				}
				else if ((red == 255) && (green == 0) && (blue == 0)) // If the color is red boundry tile
				{
					// --- Render a wall of boxes

					tilePos = glm::vec3(verticalOffset + j * -tileScale, -tileScale / 2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isBoundingTile = true;

					tilePos = glm::vec3(verticalOffset + j * -tileScale, tileScale / 2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isBoundingTile = true;
					AllGameObjects.push_back(terrainTemp);

					tilePos = glm::vec3(verticalOffset + j * -tileScale, 3.0f * tileScale / 2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isBoundingTile = true;
					AllGameObjects.push_back(terrainTemp);

					tilePos = glm::vec3(verticalOffset + j * -tileScale, 5.0f * tileScale / 2.0f, horizontalOffset - i * tileScale);
					// Make a cube game object and push it back into the array so it is drawn
					terrainTemp = make_shared<GameObject>("terrain2", cube, resourceDirectory, progTerrain, tilePos, tileOrientation, false, 0, true);
					sceneTerrainObjs.push_back(terrainTemp);
					sceneTerrainObjs[sceneTerrainObjs.size() - 1]->isBoundingTile = true;
					AllGameObjects.push_back(terrainTemp);
				}
				
			}
		}

		//need to free the dataLayout after ur done w/ it still


		// Setup new Ground plane
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 0.0f);
		// groundbox = make_shared<GameObject>("groundbox", cube, resourceDirectory, prog, position, orientation, false, 0);


		// Setup the 1st team 1 robot
		position = vec3(-5.0f, 0.1f, -50.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> PlayerBun = make_shared<GameObject>("player", maRobotShape, "../resources/", prog, position, orientation, true, 1, false);
		sceneActorGameObjs.push_back(PlayerBun);
		robotUnits.push_back(PlayerBun);
        AllGameObjects.push_back(PlayerBun);

		// Setup the 2nd team 1 robot
		position = vec3(5.0f, 0.1f, -50.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> NPCBun = make_shared<GameObject>("robot2", maRobotShape, "../resources/", prog, position, orientation, true, 1, false);
		sceneActorGameObjs.push_back(NPCBun);
		robotUnits.push_back(NPCBun);
        AllGameObjects.push_back(NPCBun);

		// Setup the 3rd team 1 robot
		position = vec3(-25.0f, 1.1f, -50.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> robot3 = make_shared<GameObject>("robot3", maRobotShape, "../resources/", prog, position, orientation, true, 1, false);
		sceneActorGameObjs.push_back(robot3);
		robotUnits.push_back(robot3);
        AllGameObjects.push_back(robot3);
		
		// Setup the 4th team 1 robot
		position = vec3(25.0f, 1.1f, -50.0f);
		orientation = vec3(0.0f, 0.0f, 1.0f);
		shared_ptr<GameObject> robot4 = make_shared<GameObject>("robot4", maRobotShape, "../resources/", prog, position, orientation, true, 1, false);
		sceneActorGameObjs.push_back(robot4);
		robotUnits.push_back(robot4);
        AllGameObjects.push_back(robot4);

		// ---- Setup the team 2 robots ----
		// Setup the 1st team 2 robot
		position = vec3(-5.0f, 0.1f, 50.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien0 = make_shared<GameObject>("alien0", maRobotShape, "../resources/", prog, position, orientation, true, 2, false);
		sceneActorGameObjs.push_back(alien0);
		alienUnits.push_back(alien0);
        AllGameObjects.push_back(alien0);
		

		// Setup the 2nd team 2 robot
		position = vec3(5.0f, 0.1f, 50.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien1 = make_shared<GameObject>("alien1", maRobotShape, "../resources/", prog, position, orientation, true, 2, false);
		sceneActorGameObjs.push_back(alien1);
		alienUnits.push_back(alien1);
        AllGameObjects.push_back(alien1);

		// Setup the 3rd team 2 robot
		position = vec3(-25.0f, 1.1f, 50.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien2 = make_shared<GameObject>("alien2", maRobotShape, "../resources/", prog, position, orientation, true, 2, false);
		sceneActorGameObjs.push_back(alien2);
		alienUnits.push_back(alien2);
        AllGameObjects.push_back(alien2);

		// Setup the 4th team 2 robot
		position = vec3(25.0f, 1.1f, 50.0f);
		orientation = vec3(0.0f, 0.0f, -1.0f);
		shared_ptr<GameObject> alien3 = make_shared<GameObject>("alien3", maRobotShape, "../resources/", prog, position, orientation, true, 2, false);
		sceneActorGameObjs.push_back(alien3);
		alienUnits.push_back(alien3);
		AllGameObjects.push_back(alien3);


		
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

	void renderGroundPlane(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, bool overheadView)
	{
		//vec3 camLoc;
		//camLoc = curCamEye;

		prog->bind();

		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(0.f, -1.f, 0.f)); //move the plane down a little bit in y space 
		M->scale(vec3(40.f, .1f, 60.f)); // Make sure that the ration is 2:3 for height to width

		groundbox->step(deltaTime, M, P, curCamEye, curCamCenter, up);
		//add uniforms to shader
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		glUniform3f(prog->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
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
		// Do logic here!
	}

	void renderSceneActors(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P, bool overheadView, int offsetX, int offsetZ)
	{	

		prog->bind(); // Bind the Simple Shader
		for (int i = 0; i < sceneActorGameObjs.size(); i++) {


			if(sceneActorGameObjs[i]->isRender == true){
				M->pushMatrix();
				M->loadIdentity();

				// Update the position of the rabbit based on velocity, time elapsed also updates the center of the bbox
				sceneActorGameObjs[i]->step(deltaTime, M, P, curCamEye, curCamCenter, up);
				// bunBun->DoCollisions()


				//add uniforms to shader
				// Set the materials of the bunny depending on if the player has hit it or not
				//if (sceneActorGameObjs[i]->beenShot)
				//{
				//	//SetMaterial(2);
				//	//M->rotate(180.0f, vec3(0, 1, 0));
				//	// glUniform1f(prog->getUniform("hit"), 1); //old method
				//}
				if (sceneActorGameObjs[i]->team == 1)
				{
					SetMaterial(3);
				}
				else if (sceneActorGameObjs[i]->team == 2)
				{
					SetMaterial(4);
				}

				// Set the color to grey if the units have been used
				if (sceneActorGameObjs[i]->isUsed == true )
				{
					SetMaterial(1);
				}

				// Only draw units that aren't dead
				if (sceneActorGameObjs[i]->health > 0.0f)
				{
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
					glUniform3f(prog->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
					glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(curCamEye, curCamCenter, up)));

					glUniform3f(prog->getUniform("lightSource"), 0, 88, 10);
					sceneActorGameObjs[i]->DrawGameObj(); // Draw the bunny model and render bbox
				}
				

				M->popMatrix();
			}
			
		}


		prog->unbind(); // Unbind the Simple Shader

		return;

	}

	void renderWeapons(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P)
	{
		static float rotato = 0.0f;
		prog->bind();

		for (int i = 0; i < weapons.size(); i++)
		{
			M->pushMatrix();
			SetMaterial(2); // Render the weapons as gold
			weapons[i]->step(deltaTime, M, P, curCamEye, curCamCenter, up);
			
			M->rotate(rotato += 0.002f, vec3(0, 1, 0)); // Make the weapons spin around
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(prog->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(curCamEye, curCamCenter, up)));

			glUniform3f(prog->getUniform("lightSource"), 0, 60, 0);
			weapons[i]->DrawGameObj(); // Draw the bunny model and render bbox

			M->popMatrix();
		}

		prog->unbind();
	}

	void renderTerrain(shared_ptr<MatrixStack> &M, shared_ptr<MatrixStack> &P)
	{
		progTerrain->bind();

		
		for (int i = 0; i < sceneTerrainObjs.size(); i++)
		{
			M->pushMatrix();
			M->loadIdentity();

			// Update the position of the rabbit based on velocity, time elapsed also updates the center of the bbox
			sceneTerrainObjs[i]->step(deltaTime, M, P, curCamEye, curCamCenter, up);

			// If terrain
			if (sceneTerrainObjs[i]->isGroundTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Floor);
				//SetMaterial(1);
				//M->scale(vec3(2.f, 2.f, 2.f));
			}
			else if (sceneTerrainObjs[i]->isUpperTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Hex);
				//SetMaterial(2);
				//M->scale(vec3(2.f, 2.f, 2.f));
			}
			else if (sceneTerrainObjs[i]->isCoverTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Wall);
				//SetMaterial(3);
				//M->scale(vec3(2.f, 2.f, 2.f));
			}
			else if (sceneTerrainObjs[i]->isJumpTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Fan);
				//SetMaterial(4);
				//M->scale(vec3(2.f, 2.f, 2.f));
			}
			else if (sceneTerrainObjs[i]->isUpperCoverTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Wall);
				//SetMaterial(3);
				//M->scale(vec3(2.f, 2.f, 2.f));
			}
			else if (sceneTerrainObjs[i]->isBoundingTile)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Tex_Wall);
			}

			glUniformMatrix4fv(progTerrain->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(progTerrain->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(progTerrain->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
			glUniformMatrix4fv(progTerrain->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt(curCamEye, curCamCenter, up)));

			glUniform3f(progTerrain->getUniform("lightSource"), 0, 88, 10);
			sceneTerrainObjs[i]->DrawGameObj(); // Draw the bunny model and render bbox
			M->popMatrix();
		}

		progTerrain->unbind();
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

		return;
	}

	void jumpPad(shared_ptr<GameObject> currObjectPointer) {
		vec3 orient;
		orient.x = radius * cos(phi)*cos(theta);
		orient.y = radius * sin(phi);
		orient.z = radius * cos(phi)*sin(theta);


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

		// Checks if in FPS mode
		if (!isOverheadView) {
			//turnStartTime == 0 means that it is the start of a turn
			if (turnStartTime == 0) {
				turnStartTime = glfwGetTime();
			}
			else if (glfwGetTime() - turnStartTime > turnLength)
			{
				//switchTurn();
				readyToSwitch = true;
			}
		}
	}

	void switchTurn() {
		if (whoseTurn == 1) {
			// if all units used clear array and allow them to be used again
			if (usedRobotUnits.size() == numRobotUnits) {
				for(int i = 0; i < usedRobotUnits.size(); i++){
					usedRobotUnits[i]->isUsed = false;
				}
				usedRobotUnits.clear();
			}
			// switch turn
			whoseTurn = 2;
		}
		else if (whoseTurn == 2) {
			// if all units used clear array and allow them to be used again
			if (usedAlienUnits.size() == numAlienUnits) {
				
				//Walkthrough used array to set bools back to unused
				for(int i = 0; i < usedAlienUnits.size(); i++){
					usedAlienUnits[i]->isUsed = false;
				}
				
				usedAlienUnits.clear();
			}
			// switch turn
			whoseTurn = 1;
		}

		//Snap user back to overhead view
		isOverheadView = true;
		firstPersonUI.setRender(false);
		overViewUI.setRender(true);
		//reset turn timer
		turnStartTime = 0;
		readyToSwitch = false;
	}

	bool GravityGroundCollision(vec3 futurePosition){
		if(possessedActor != NULL){

			//Check Collisions, Might disable this for performance increase
			for(int i = 0; i < sceneTerrainObjs.size(); i++){
				bool collisionX = futurePosition.x + possessedActor->bboxSize.x >= sceneTerrainObjs[i]->bboxCenter.x && sceneTerrainObjs[i]->bboxCenter.x + sceneTerrainObjs[i]->bboxSize.x >= futurePosition.x;
				bool collisionY = futurePosition.y + possessedActor->bboxSize.y >= sceneTerrainObjs[i]->bboxCenter.y && sceneTerrainObjs[i]->bboxCenter.y + sceneTerrainObjs[i]->bboxSize.y >= futurePosition.y;
				bool collisionZ = futurePosition.z + possessedActor->bboxSize.z >= sceneTerrainObjs[i]->bboxCenter.z && sceneTerrainObjs[i]->bboxCenter.z + sceneTerrainObjs[i]->bboxSize.z >= futurePosition.z;

				if(collisionX && collisionY && collisionZ){
					return collisionX && collisionY && collisionZ;
				}
			}
			return false;
		}
	}

	void ApplyGravity()
	{
		if(possessedActor != NULL)
		{
			float NewY = possessedActor->position.y + (velocity * deltaTime);
			vec3 NewPosition = vec3(possessedActor->position.x, NewY, possessedActor->position.z);
			if (!GravityGroundCollision(NewPosition)) {
				possessedActor->position.y = NewY;
			}
			else{
				velocity = 0.0f;
				canJump = true;
				if(readyToSwitch == true){
				    switchTurn();
				}
			}
		}
	}

	void renderUI() {
		//start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (mainMenuUI.shouldRender())
			mainMenuUI.drawAll();

		else if (overViewUI.shouldRender()) {
			ImGui::Begin("over view");
			ImGui::SetWindowSize(ImVec2(WINDOWSIZE_X, WINDOWSIZE_Y / 8));
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGuiWindowFlags_NoBackground;
			ImGuiWindowFlags_NoMove;

			ImGui::Text("Take Your Turn Player %d !!!", whoseTurn);

			if (whoseTurn == 1) {
				ImGui::Text("You have %d robots left", numRobotUnits - usedRobotUnits.size());
			}

			else if (whoseTurn == 2) {
				ImGui::Text("You have %d aliens left", numAlienUnits - usedAlienUnits.size());
			}

			for (int i=0; i < sceneActorGameObjs.size(); i++) {
				ImGui::Text("%s HP: %f", sceneActorGameObjs[i]->nameObj.c_str(), sceneActorGameObjs[i]->health);
			}


			ImGui::End();
			
			//update and use this after
			//overViewUI.drawAll();
		}

		else if (firstPersonUI.shouldRender()) {
			ImGui::Begin("first person");
			ImGui::SetWindowSize(ImVec2(WINDOWSIZE_X, WINDOWSIZE_Y / 8));
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGuiWindowFlags_NoBackground;
			ImGuiWindowFlags_NoMove;

			float curTime = glfwGetTime();
			float timeLeft = turnLength - (curTime - turnStartTime);
			ImGui::Text("Time Left: %1.f", timeLeft);

			//health bar stuff
			//get cur player health, input max and min values
			//ImGui::ProgressBar(fraction, ImVec2(0, 50), HP);

			ImGui::End();

			//should be used, will update when fixed
			//firstPersonUI.drawAll();
		}

		if (DEBUG_MODE) {
			//TODO: if time permitting, add more debug features
			ImGui::Begin("Developer Toolbox");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
			// printf("MS/FPS: %f    FPS: %f\n", 1000.0 / double(nbFrames) , double(nbFrames)); // Print out ms/fps and frame rate
			//printf("Objects on the ground: %d\n", sceneActorGameObjs.size()); // Print out the number of objects in the game
			//printf("Objects Collided with %d\n\n", p1Collisions);
			nbFrames = 0;
			lastTime += 1.0;
		}

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		elapsedTime += deltaTime;		


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

		// Check if the camera should be interpolated
		static float camInterp = 0.0f;
		if (camInterp > 1.0f && camUpdate)
		{
			camInterp = 0.0f;
			camUpdate = false;

			
			isOverheadView = false; // Toggle the currentCamera after interpolation is finished
			firstPersonUI.setRender(true);
			overViewUI.setRender(false);
		}

		if (camInterp <= 1.0f && camUpdate) // interpolate from the overhead camera to the possesd gameobjects view
		{
			camInterp += 1.f * deltaTime;
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

		
		
		
		// renderGroundPlane(M, P, isOverheadView); //draw the ground plane

		//acceleration = -9.8f;

		// Update the position of the players bbox
		// renderPlayerBbox(M, P, isOverheadView);

		velocity += deltaTime * acceleration;

		ApplyGravity();

		M->pushMatrix();
		//checkAllGameObjects();
		renderSceneActors(M, P, isOverheadView, 0, 0); // render all the actors in the scene
		renderTerrain(M, P); // Render all objs in the terrain
		renderWeapons(M, P);
		checkAllGameObjects();
		//bunBun->DoCollisions(groundbox);
		M->popMatrix();

		// ----- Draw the weapon in first person ------
		if (!isOverheadView)
		{
			// Draw the Gun
			glClear(GL_DEPTH_BUFFER_BIT); // Clear the depth buffer so the weapon is drawn over all other geometry
			if (possessedActor != NULL && possessedActor->currWeapon == 0) // if the weapon is a pistol
			{
				M->pushMatrix();
				prog->bind();
				M->translate(vec3(0.8f, -0.8f, -1.9f));
				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				glUniform3f(prog->getUniform("lightSource"), 0, 0, 0);
				SetMaterial(1); // Flat Grey
				gun->draw(prog);
				M->popMatrix();
			}
			else if (possessedActor != NULL && possessedActor->currWeapon == 1) // if the weapon is a shotgun
			{
				M->pushMatrix();
				prog->bind();
				M->translate(vec3(1.0f, -0.3f, -1.1f));
				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				glUniform3f(prog->getUniform("lightSource"), 0, 0, 0);
				SetMaterial(1); // Flat Grey
				shotgun->draw(prog);
				M->popMatrix();
			}
			

			// Draw the Crosshair
			M->pushMatrix();
			SetMaterial(5); //Red
			// Draw the horizontal line
			M->translate(vec3(0.0f, 0.0f, -15.0f));
			M->scale(vec3(0.5f, 0.1f, 0.1f));
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(prog->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(prog->getUniform("lightSource"), 0, 0, 0);
			cube->draw(prog);
			M->popMatrix();

			// Draw the vertical line
			M->pushMatrix();
			M->translate(vec3(0.0f, 0.0f, -15.0f));
			M->scale(vec3(0.1f, 0.5f, 0.1f));
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(prog->getUniform("eye"), curCamEye.x, curCamEye.y, curCamEye.z);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			glUniform3f(prog->getUniform("lightSource"), 0, 0, 0);
			cube->draw(prog);
			M->popMatrix();

			prog->unbind();
			
		}

		renderUI();

		M->popMatrix(); // Pop Scene Matrix
		P->popMatrix(); // This wasnt here b4
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
	application->initUI(windowManager->getHandle());

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

	//teardown Imgui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
