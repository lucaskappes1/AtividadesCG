#include "Object.h"
#include <stb_image.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <GLFW/glfw3.h>
#include "Camera.h"

Object::Object()
{
	
}

glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * ((2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}


void Object::loadGeometry(const char* filepath, GLuint shaderID)
{
	std::vector<GLfloat> vertices;
	std::vector<glm::vec3> v;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::ifstream file(filepath);

	if (!file)
	{
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return;
	}

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::string line;

	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "v")
		{
			glm::vec3 vertex;
			iss >> vertex.x >> vertex.y >> vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (type == "vt")
		{
			glm::vec2 uv;
			iss >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (type == "vn")
		{
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (type == "f")
		{
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			char slash;

			for (int i = 0; i < 3; ++i)
			{
				iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
				vertexIndices.push_back(vertexIndex[i]);
				uvIndices.push_back(uvIndex[i]);
				normalIndices.push_back(normalIndex[i]);
			}
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		v.push_back(vertex);
		uvs.push_back(uv);
		normals.push_back(normal);
	}

	file.close();

	vertices.reserve(v.size() * 8);
	for (size_t i = 0; i < v.size(); ++i)
	{
		vertices.insert(vertices.end(), {
			v[i].x, v[i].y, v[i].z,
			1.0f, 0.0f, 0.0f,
			uvs[i].x, uvs[i].y
			});
	}

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	this->VAO = VAO;
	this->vertexCount = vertices.size() / 6;


	std::string basePath = std::string(filepath).substr(0, std::string(filepath).find_last_of("/"));
	std::string filenameNoExt = std::string(filepath).substr(std::string(filepath).find_last_of("/") + 1);
	filenameNoExt = filenameNoExt.substr(0, filenameNoExt.find_last_of("."));

	std::string mtlPath = basePath + "/" + filenameNoExt + ".mtl";

	std::ifstream mtlFile(mtlPath);
	if (!mtlFile)
	{
		std::cerr << "Failed to open MTL file: " << mtlPath << std::endl;
		return;
	}

	std::string templine, texturePath;
	glm::vec3 ka(0.0f);
	glm::vec3 kd(0.0f);
	glm::vec3 ks(0.0f);
	float ns = 32.0f;
	while (getline(mtlFile, templine))
	{
		std::istringstream iss(templine);
		std::string keyword;
		iss >> keyword;

		if (keyword == "Ka")
		{
			iss >> ka.r >> ka.g >> ka.b;
		}
		else if (keyword == "Kd")
		{
			iss >> kd.r >> kd.g >> kd.b;
		}
		else if (keyword == "Ks")
		{
			iss >> ks.r >> ks.g >> ks.b;
		}
		else if (keyword == "Ns")
		{
			iss >> ns;
		}
		else if (keyword == "map_Kd")
		{
			iss >> texturePath;
		}
	}
	mtlFile.close();
	GLint kaLoc = glGetUniformLocation(shaderID, "ka");
	GLint kdLoc = glGetUniformLocation(shaderID, "kd");
	GLint ksLoc = glGetUniformLocation(shaderID, "ks");
	GLint qLoc = glGetUniformLocation(shaderID, "q");
	if (kaLoc != -1) glUniform3f(kaLoc, ka.r, ka.g, ka.b);
	if (kdLoc != -1) glUniform3f(kdLoc, kd.r, kd.g, kd.b);
	if (ksLoc != -1) glUniform3f(ksLoc, ks.r, ks.g, ks.b);
	if (qLoc != -1) glUniform1f(qLoc, ns);


	if (texturePath.empty())
	{
		std::cerr << "No diffuse texture found in MTL file: " << mtlPath << std::endl;
	}
	std::string textureFile = texturePath;

	if (!textureFile.empty())
	{
		std::string fullTexturePath = basePath + "/" + textureFile;


		GLuint texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(fullTexturePath.c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			std::cout << "Loaded texture: " << fullTexturePath << " (" << width << "x" << height << ")" << std::endl;
		}

		if (data)
		{
			if (nrChannels == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);
		this->textureID = texID;
		this->textureFilePath = fullTexturePath;
	}
	return;
}

void Object::draw(GLint modelLoc)
{
	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, m_position);
	model = glm::rotate(model, m_rotateAngle.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, m_rotateAngle.y, glm::vec3(0.0f, 1.0f, 0.0f));	
	model = glm::rotate(model, m_rotateAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, m_scale);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Object::update(GLFWwindow* window, Camera* camera)
{
	if (m_waypoints.empty())
	{
		return;
	}
	t += speed;
	if (t >= 1.0f)
	{
		t = 0.0f;
		currentWaypoint = (currentWaypoint + 1) % m_waypoints.size();
	}

	// Pegando os 4 pontos para Catmull-Rom
	
	int p0 = (currentWaypoint - 1 + m_waypoints.size()) % m_waypoints.size();
	int p1 = currentWaypoint;
	int p2 = (currentWaypoint + 1) % m_waypoints.size();
	int p3 = (currentWaypoint + 2) % m_waypoints.size();

	m_position = catmullRom(
		m_waypoints[p0],
		m_waypoints[p1],
		m_waypoints[p2],
		m_waypoints[p3],
		t
	);
}

void Object::deleteVertexArray()
{
	glDeleteVertexArrays(1, &VAO);
}

void Object::addWaypoint(const glm::vec3& waypoint)
{
	m_waypoints.push_back(waypoint);
}

void Object::processInput(GLFWwindow* window, Camera* camera)
{
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		if (!addWaypointKeyPressed)
		{
			m_waypoints.push_back(camera->getPosition());
			addWaypointKeyPressed = true;
			std::cout << "Waypoint adicionado: " << camera->getPosition().x << ", " << camera->getPosition().y << ", " << camera->getPosition().z << std::endl;
		}
	}
	else
	{
		addWaypointKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		m_rotateAngle.z += 0.01f;
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		m_rotateAngle.x += 0.01f;
	}

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		m_rotateAngle.y += 0.01f;
	}
}
