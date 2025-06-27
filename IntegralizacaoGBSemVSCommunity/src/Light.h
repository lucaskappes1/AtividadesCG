#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
	Light(const glm::vec3& position, const glm::vec3& color, GLuint shaderID, int lightNumber);
	void setLightIntensity(const glm::vec3& color);
	glm::vec3 getLightIntensity() const;
private:
	glm::vec3 m_position;
	glm::vec3 m_color;
	GLuint m_shaderID;
	int m_lightNumber;
};

