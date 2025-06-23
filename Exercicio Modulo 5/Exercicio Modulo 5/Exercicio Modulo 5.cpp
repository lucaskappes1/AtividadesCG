#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

using namespace std;

struct Geometry
{
	GLuint VAO;
	GLuint vertexCount;
	GLuint textureID = 0;
	string textureFilePath;
};

// Protótipos das funções
int setupShader();
Geometry loadGeometry(const char* filepath, GLuint shaderID);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texc;
layout (location = 3) in vec3 normal;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
out vec2 texCoord;
out vec3 vNormal;
out vec4 fragPos;
out vec4 vertexColor;
void main()
{
   	gl_Position =  projection * view * model * vec4(position.x, position.y, position.z, 1.0);
	fragPos = model * vec4(position.x, position.y, position.z, 1.0);
	vertexColor = vec4(color, 1.0);
	vNormal = normal;
	texCoord = vec2(texc.x, 1 - texc.y);
})";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = R"(
#version 450

in vec2 texCoord;
in vec4 vertexColor;
in vec4 fragPos;
in vec3 vNormal;

out vec4 color;

uniform sampler2D tex_buffer;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform vec3 lightColor;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;

void main()
{
    vec3 ambient = lightColor * ka;
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vec3(fragPos));
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * kd;
    vec3 R = reflect(-L, N);
    vec3 V = normalize(camPos - vec3(fragPos));
    float spec = pow(max(dot(R, V), 0.0), q);
    vec3 specular = spec * ks * lightColor;
    vec3 texColor = texture(tex_buffer, texCoord).rgb;
    vec3 result = (ambient + diffuse) * texColor + specular;
    color = vec4(result, 1.0f);
})";


bool rotateX = false, rotateY = false, rotateZ = false;

glm::vec3 translate_vector = {0.0, 0.0, 0.0};
glm::vec3 scale_vector = { 1.0, 1.0, 1.0 };
glm::vec3 lightPos = { 3.0, 6.0, 0.2 };
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
glm::vec3 lightColor = { 1.0, 1.0, 1.0 };

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Lucas Kappes!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported " << version << std::endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	glUseProgram(shaderID);
	Geometry g = loadGeometry("../assets/Modelos3D/Suzanne.obj", shaderID);

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	GLint lightPosLoc = glGetUniformLocation(shaderID, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	GLint lightColorLoc = glGetUniformLocation(shaderID, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	camera.initCamera(shaderID);

	glfwSetCursorPosCallback(window, mouse_callback);
	// Esconde o cursor e captura ele
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		camera.update(window);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1);
		model = glm::translate(model, translate_vector + glm::vec3(0.0, 0.25, 0.0));
		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		model = glm::scale(model, scale_vector);

		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));
		glBindTexture(GL_TEXTURE_2D, g.textureID);
		glBindVertexArray(g.VAO);
		glDrawArrays(GL_TRIANGLES, 0, g.vertexCount);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &g.VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		translate_vector += glm::vec3(0.0, 0.1, 0.0);
	}

	if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		translate_vector += glm::vec3(0.0, -0.1, 0.0);
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		scale_vector += glm::vec3(0.1, 0.1, 0.1);
	}

	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		scale_vector += glm::vec3(-0.1, -0.1, -0.1);
	}

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.mouseCallback(xpos, ypos);
}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

Geometry loadGeometry(const char* filepath, GLuint shaderID)
{
	std::vector<GLfloat> vertices;
	std::vector<glm::vec3> v;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::ifstream file(filepath);

	if (!file)
	{
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return Geometry{};
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

	Geometry geometry;
	geometry.VAO = VAO;
	geometry.vertexCount = vertices.size() / 6;

	string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
	string filenameNoExt = string(filepath).substr(string(filepath).find_last_of("/") + 1);
	filenameNoExt = filenameNoExt.substr(0, filenameNoExt.find_last_of("."));

	string mtlPath = basePath + "/" + filenameNoExt + ".mtl";

	ifstream mtlFile(mtlPath);
	if (!mtlFile)
	{
		cerr << "Failed to open MTL file: " << mtlPath << endl;
		return Geometry{};
	}

	string templine, texturePath;
	glm::vec3 ka(0.0f);
	glm::vec3 kd(0.0f);
	glm::vec3 ks(0.0f);
	float ns = 32.0f;
	while (getline(mtlFile, templine))
	{
		istringstream iss(templine);
		string keyword;
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
		cerr << "No diffuse texture found in MTL file: " << mtlPath << endl;
	}
	string textureFile = texturePath;

	if (!textureFile.empty())
	{
		string fullTexturePath = basePath + "/" + textureFile;


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
		geometry.textureID = texID;
		geometry.textureFilePath = fullTexturePath;
	}
	return geometry;
}