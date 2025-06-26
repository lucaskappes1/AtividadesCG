#include "Light.h"
#include <string>

Light::Light(const glm::vec3& position, const glm::vec3& color, GLuint shaderID, int lightNumber) : m_position(position), m_color(color) 
{
    std::string base = "lights[" + std::to_string(lightNumber) + "].";
    glUniform3fv(glGetUniformLocation(shaderID, (base + "position").c_str()), 1, glm::value_ptr(m_position));
    glUniform3fv(glGetUniformLocation(shaderID, (base + "color").c_str()), 1, glm::value_ptr(m_color));
}
