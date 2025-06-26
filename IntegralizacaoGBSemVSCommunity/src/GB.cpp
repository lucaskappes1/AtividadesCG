#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneLoader.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

using namespace std;

// Protótipos das funções
int setupShader();

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

#define MAX_LIGHTS 16

struct Light {
    vec3 position;
    vec3 color;
};

uniform int numLights;
uniform Light lights[MAX_LIGHTS];

uniform vec3 camPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;

uniform sampler2D tex_buffer;

in vec2 texCoord;
in vec4 vertexColor;
in vec4 fragPos;
in vec3 vNormal;

out vec4 color;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(camPos - vec3(fragPos));
    vec3 texColor = texture(tex_buffer, texCoord).rgb;
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < numLights; ++i)
    {
        vec3 L = normalize(lights[i].position - vec3(fragPos));
        vec3 R = reflect(-L, N);

        // Ambient
        ambient += lights[i].color * ka;

        // Diffuse
        float diff = max(dot(N, L), 0.0);
        diffuse += diff * lights[i].color * kd;

        // Specular
        float spec = pow(max(dot(R, V), 0.0), q);
        specular += spec * ks * lights[i].color;
    }

    vec3 result = (ambient + diffuse) * texColor + specular;
    color = vec4(result, 1.0);
}
)";
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));

std::vector<Object> objects;
std::vector<Light> lights;
int selectedObject = 0;
bool isUpdatingObjects = false;

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
	SceneLoader sceneLoader(shaderID);
	objects = sceneLoader.loadObjects("../config/scene_objects_config.txt");

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	lights = sceneLoader.loadLights("../config/scene_lights_config.txt");
	// Envia quantidade de luzes
	glUniform1i(glGetUniformLocation(shaderID, "numLights"), lights.size());

	camera = sceneLoader.loadCamera("../config/scene_camera_config.txt");

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
		objects[selectedObject].processInput(window, &camera);
		for(auto& object : objects) 
		{
			isUpdatingObjects = true;
			object.update(window, &camera);
			isUpdatingObjects = false;
			object.draw(modelLoc);
		}

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	for(auto& object : objects) 
	{
		object.deleteVertexArray();
	}
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

	if(key == GLFW_KEY_PERIOD && action == GLFW_PRESS) 
	{
		selectedObject = (selectedObject + 1) % objects.size();
		std::cout << "Selected object: " << selectedObject << std::endl;
	}

	if (key == GLFW_KEY_COMMA && action == GLFW_PRESS)
	{
		selectedObject = (selectedObject - 1) % objects.size();
		std::cout << "Selected object: " << selectedObject << std::endl;
	}

	if(key == GLFW_KEY_UP && action == GLFW_PRESS) 
	{
		if(!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(0.0f, 0.0f, -0.1f));
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		if (!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(0.0f, 0.0f, 0.1f));
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		if (!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(-0.1f, 0.0f, 0.0f));
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		if (!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(0.1f, 0.0f, 0.0f));
	}

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		if (!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(0.0f, 0.1f, 0.0f));
	}

	if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		if (!isUpdatingObjects)
			objects[selectedObject].clearWaypoints();
		objects[selectedObject].setPosition(objects[selectedObject].getPosition() + glm::vec3(0.0f, -0.1f, -0.0f));
	}

	if(key == GLFW_KEY_O && action == GLFW_PRESS) 
	{
		objects[selectedObject].setScale(objects[selectedObject].getScale() + glm::vec3(0.1f, 0.1f, 0.1f));
	}

	if (key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		objects[selectedObject].setScale(objects[selectedObject].getScale() + glm::vec3(-0.1f, -0.1f, -0.1f));
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
