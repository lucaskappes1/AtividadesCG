#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
	Light(const glm::vec3& position, const glm::vec3& color, GLuint shaderID, int lightNumber);

private:
	glm::vec3 m_position;
	glm::vec3 m_color;
};

