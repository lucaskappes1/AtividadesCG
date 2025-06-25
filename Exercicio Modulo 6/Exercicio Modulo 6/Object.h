#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

class Object
{
public:
	Object();
	void loadGeometry(const char* filepath, GLuint shaderID);
	
	void draw(GLint modelLoc);
	void update(struct GLFWwindow* window, class Camera* camera);

	GLuint getVAO() const { return VAO; }
	GLuint getVertexCount() const { return vertexCount; }
	GLuint getTextureID() const { return textureID; }
	glm::vec3 getPosition() const { return m_position; }
	glm::vec3 getScale() const { return m_scale; }
	void deleteVertexArray();
private:
	void setWaypoints();
	void processInput(struct GLFWwindow* window, class Camera* camera);
	int actualWaypoint = 0;
	size_t currentWaypoint = 0;
	float t = 0.0f;          
	float speed = 0.01f;       
	bool addWaypointKeyPressed = false;


	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	std::vector<glm::vec3> m_waypoints;
	GLuint VAO;
	GLuint vertexCount;
	GLuint textureID = 0;
	std::string textureFilePath;
};

