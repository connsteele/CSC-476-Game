#include "GameObject.h"

struct Particle {
	glm::vec3 pos;
	glm::vec4 color;
	GLfloat life, vel;

	Particle() : pos(0.f), vel(0.f), color(1.f), life(0.f) {}
};

class ParticleGenerator
{
public:
	// Constructor
	ParticleGenerator(std::shared_ptr<Program> shader, GLuint texture, GLuint amount);
	// Update all particles
	void Update(GLfloat dt, std::shared_ptr<GameObject>, GLuint newParticles, glm::vec3 offset = glm::vec3(0.f));
	// Render all particles
	void Draw();
private:
	// State
	std::vector<Particle> particles;
	GLuint amount;
	// Render state
	std::shared_ptr<Program> shader;
	GLuint texture;
	GLuint VAO;
	// Initializes buffer and vertex attributes
	void init();
	// Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
	GLuint firstUnusedParticle();
	// Respawns particle
	void respawnParticle(Particle &particle, std::shared_ptr<GameObject>, glm::vec3 offset = glm::vec3(0.f));
};