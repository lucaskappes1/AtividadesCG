#pragma once
#include "Object.h"
#include "Light.h"
#include "Camera.h"
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class SceneLoader
{
public:
	SceneLoader(GLuint shaderID) : shaderID(shaderID), m_camera(glm::vec3(0.0f, 0.0f, 0.0f)) {}
	std::vector<Object>& loadObjects(const std::string& filePath);
	std::vector<Light>& loadLights(const std::string& filePath);
	Camera& loadCamera(const std::string& filePath);
private:
	std::vector<Object> m_objects;
	std::vector<Light> m_lights;
	Camera m_camera;
	GLuint shaderID;
};