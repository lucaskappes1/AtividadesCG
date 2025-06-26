#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLAD/glad.h>

class Camera
{
public:
	Camera(glm::vec3 position);
	void setPosition(glm::vec3 position) { m_position = position; }
	void setLookAt(glm::vec3 lookAt) { m_lookAt = lookAt; }
	void mouseCallback(double xpos, double ypos);
	void initCamera(GLuint shaderID);
	void update(struct GLFWwindow* window);
	glm::vec3 getPosition() const { return m_position; }
	glm::vec3 getLookAt() const { return m_lookAt; }
	glm::vec3 getCameraUp() const { return m_cameraUp; }
	void setFrustum(float fov, float aspectRatio, float nearPlane, float farPlane);
private:
	void processInput(struct GLFWwindow* window);
	glm::vec3 m_position;
	glm::vec3 m_lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_fov = 45.0f;
	float m_aspecRatio = 1.33f;
	float m_nearPlane = 0.1f;
	float m_farPlane = 100.0f;

	GLint m_viewLoc;
	GLuint m_shaderID;

	// Para controle de �ngulo
	float yaw = -90.0f;  // Come�a olhando no -Z
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;  // Metade da largura da janela
	float lastY = 600.0f / 2.0;  // Metade da altura da janela
	bool firstMouse = true;

	// Sensibilidade do mouse
	float sensitivity = 0.1f;
};

