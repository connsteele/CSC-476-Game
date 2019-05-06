#include "GameObject.h"

// Connor Steele and Chris Gix Game Object Implementation


GameObject::GameObject(const std::string& gameObjName, std::shared_ptr<Shape>& objModel, const std::string& resourceDirectory, std::shared_ptr<Program> curShaderProg, glm::vec3 pos, glm::vec3 orient, bool visibleBbox, int team, bool isStaticGeom)
{
	this->nameObj = gameObjName;
	this->objModel = objModel;
	this->hitByPlayer = false;

	//-- Setup geometry of the bounding box
	initBbox();


	//Initialize globals
	this->curShaderProg = curShaderProg;
	this->position = pos;
	this->orientation = orient;
	this->visibleBbox = visibleBbox;
	this->beenShot = false;
	this->isGroundTile = false;
	this->isUpperTile = false;
	this->isCoverTile = false;
	this->isJumpTile = false;
	this->isUpperCoverTile = false;
	this->isStaticGeom = isStaticGeom;

	this->team = team;
	this->currWeapon = 0;

	elapsedTime = 0.0f;

	//Set global bools
	isPosessed = false;
	isRender = true;
}

void GameObject::DrawGameObj()
{
	objModel->draw(curShaderProg);
	renderBbox();
}

void GameObject::step(float dt, std::shared_ptr<MatrixStack> &M, std::shared_ptr<MatrixStack> &P, glm::vec3 camLoc, glm::vec3 center, glm::vec3 up)
{ 
	// elapsedTime += dt;

	DoCollisions(M);

	//-- Calculate new position and translate the model
	position += 0.0f * orientation * dt;
	//printf("Obj Current Pos: x: %f y: %f z: %f\n", position.x, position.y, position.z);
	M->translate(position);

	//-- Check the orientation of the game object and rotate the mesh to match its orientation
	if (orientation == glm::vec3(0, 0, -1))
	{
		M->rotate(180.0f, glm::vec3(0, 1, 0));
	}
	else if (orientation == glm::vec3(0, 0, 1))
	{
		M->rotate(0.0f, glm::vec3(0, 1, 0));
	}

	//--- Recompute bbox center and transformation matrix
	bboxCenter = position; // used to be glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2)
	bboxTransform = glm::translate(glm::mat4(1), bboxCenter) * glm::scale(glm::mat4(1), bboxSize);
	
	

}

// Check for exiting ground plane, if so flip the orientation of the bunny
void GameObject::DoCollisions(std::shared_ptr<MatrixStack> &M) //std::shared_ptr<GameObject> world
{

	//if (position.z > 40.0f || position.z < -40.0f || position.x > 40.0f || position.x < -40.0f)
	//{
	//	// Invert the z component of the orientation vector
	//	this->orientation = glm::vec3(orientation.x, orientation.y, -1.f * orientation.z);

	//}
	
}

// Update the center of the bounding box for the model
void GameObject::renderBbox()
{
	if (visibleBbox)
	{ 
		glUniformMatrix4fv(curShaderProg->getUniform("M"), 1, GL_FALSE, value_ptr(bboxTransform));
		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,  // attribute
			4,                  // number of elements per vertex, here (x,y,z,w)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

// Might be able to move this somewhere else so it doesn't send the data to the GPU again and again
void GameObject::initBbox()
{
	// Cube 1x1x1, centered on origin
	GLfloat vertices[] = {
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
	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind

	GLushort elements[] = {
	  0, 1, 2, 3,
	  4, 5, 6, 7,
	  0, 4, 1, 5, 2, 6, 3, 7
	};
	//GLuint ibo_elements;
	glGenBuffers(1, &ibo_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind

	

	// Get the position buffer from the model
	std::vector<float> modelVertPosBuf = objModel->getPosBuf();

	// Set up initial Values
	min_x = max_x = modelVertPosBuf[0];
	min_y = max_y = modelVertPosBuf[1];
	min_z = max_z = modelVertPosBuf[2];

	// Fit mins and maxes to the models vertices
	for (size_t i = 0; i < modelVertPosBuf.size() / 3; i++)
	{
		if (modelVertPosBuf[3 * i + 0] < min_x) min_x = modelVertPosBuf[3 * i + 0];
		if (modelVertPosBuf[3 * i + 0] > max_x) max_x = modelVertPosBuf[3 * i + 0];

		if (modelVertPosBuf[3 * i + 1] < min_y) min_y = modelVertPosBuf[3 * i + 1];
		if (modelVertPosBuf[3 * i + 1] > max_y) max_y = modelVertPosBuf[3 * i + 1];

		if (modelVertPosBuf[3 * i + 2] < min_z) min_z = modelVertPosBuf[3 * i + 2];
		if (modelVertPosBuf[3 * i + 2] > max_z) max_z = modelVertPosBuf[3 * i + 2];
	}

	bboxSize = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
	//bboxCenter = glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2); // DONT INITIALIZE TO THIS VALUE BC IT WILL MAKE THE BOX GO THE CENTER OF THE WORLD FOR A TINY BIT, ENUFF TO CAUSE COLLISION DETECTION TO TRIGGER
	bboxCenter = position; // set the center of the box to the position of the game object
	bboxTransform = glm::translate(glm::mat4(1), bboxCenter) * glm::scale(glm::mat4(1), bboxSize);

}


GameObject::~GameObject()
{
}

bool GameObject::FirePistol(glm::vec3 rayDir, std::shared_ptr<GameObject> currObjectPointer, glm::vec3 curCamCenter){
    //logic for default pistol

    bool isClicked = RayTraceCamera(rayDir, currObjectPointer, curCamCenter);
    return isClicked;
}

bool GameObject::FireShotgun(glm::vec3 rayDirCent, glm::vec3 rayDirLeft, glm::vec3 rayDirRight, glm::vec3 rayDirDown, glm::vec3 rayDirUp, std::shared_ptr<GameObject> currObjectPointer, glm::vec3 curCamCenter){
    //logic for default pistol

    bool isShotCenter = RayTraceCamera(rayDirCent, currObjectPointer, curCamCenter);
    bool isShotLeft = RayTraceCamera(rayDirLeft, currObjectPointer, curCamCenter);
    bool isShotRight = RayTraceCamera(rayDirRight, currObjectPointer, curCamCenter);
    bool isShotDown = RayTraceCamera(rayDirDown, currObjectPointer, curCamCenter);
    bool isShotUp = RayTraceCamera(rayDirUp, currObjectPointer, curCamCenter);
    return isShotCenter || isShotLeft || isShotRight || isShotDown || isShotUp;
}

bool GameObject::RayTraceCamera(glm::vec3 rayDir, std::shared_ptr<GameObject> currObjectPointer, glm::vec3 curCamCenter){
    glm::vec3 dirfrac = glm::vec3(1.0f / rayDir.x, 1.0f / rayDir.y, 1.0f / rayDir.z);

    GameObject currObject = *currObjectPointer;

    glm::vec3 lb = currObject.bboxCenter - (currObject.bboxSize / 2.0f);
    glm::vec3 rt = currObject.bboxCenter + (currObject.bboxSize / 2.0f);

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