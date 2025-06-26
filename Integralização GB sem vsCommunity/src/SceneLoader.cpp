#include "SceneLoader.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>

std::vector<Object>& SceneLoader::loadObjects(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << std::endl;
        return m_objects;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        Object obj;

        // Nome do modelo
        std::string completePath = "../assets/Modelos3D/" + line;
        obj.loadGeometry(completePath.c_str(), shaderID);

        // Função auxiliar para ler uma linha no formato: "prefix x, y, z"
        auto parseVec3Line = [](const std::string& line, const std::string& expectedPrefix) -> glm::vec3
            {
                std::istringstream iss(line);
                std::string prefix;
                float x, y, z;
                char comma1, comma2;

                iss >> prefix >> x >> comma1 >> y >> comma2 >> z;

                if (prefix != expectedPrefix || comma1 != ',' || comma2 != ',')
                {
                    throw std::runtime_error("Erro ao fazer parsing da linha: " + line);
                }

                return glm::vec3(x, y, z);
            };

        try
        {
            // Leitura das transformações básicas
            if (std::getline(file, line))
            {
                glm::vec3 rot = parseVec3Line(line, "rot");
                obj.setRotateAngle(rot);
            }

            if (std::getline(file, line))
            {
                glm::vec3 pos = parseVec3Line(line, "trans");
                obj.setPosition(pos);
            }

            if (std::getline(file, line))
            {
                glm::vec3 scale = parseVec3Line(line, "scale");
                obj.setScale(scale);
            }

            // Leitura dos waypoints (linhas que começam com "waypoint")
            while (std::getline(file, line))
            {
                if (line.rfind("waypoint", 0) == 0) // Verifica se começa com "waypoint"
                {
                    glm::vec3 waypoint = parseVec3Line(line, "waypoint");
                    obj.addWaypoint(waypoint);
                }
                else
                {
                    // Linha não é waypoint, então é o início de outro objeto ou fim
                    break;
                }
            }

            // Se lemos uma linha que não era waypoint, ela já faz parte do próximo objeto
            if (!line.empty())
            {
                // Reposiciona o cursor para a linha que foi lida mas não processada
                file.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Erro ao processar objeto " << completePath << ": " << e.what() << std::endl;
            continue; // Pula esse objeto e continua com o próximo
        }

        m_objects.push_back(obj);
    }

    file.close();
    return m_objects;
}

std::vector<Light>& SceneLoader::loadLights(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de luzes: " << filePath << std::endl;
        return m_lights;
    }

    std::string line;
    int lightNumber = 0;

    while (std::getline(file, line)) {
        // Ignorar linhas vazias ou comentários
        if (line.empty() || line[0] == '#')
            continue;

        glm::vec3 position, color;

        try {
            // Parse da posição
            {
                std::istringstream iss(line);
                std::string prefix;
                float x, y, z;
                char comma1, comma2;

                iss >> prefix >> x >> comma1 >> y >> comma2 >> z;

                if (prefix != "position" || comma1 != ',' || comma2 != ',') {
                    throw std::runtime_error("Erro ao fazer parsing da posição: " + line);
                }

                position = glm::vec3(x, y, z);
            }

            // Ler próxima linha (cor)
            if (!std::getline(file, line)) {
                throw std::runtime_error("Esperado linha de cor após posição.");
            }

            {
                std::istringstream iss(line);
                std::string prefix;
                float r, g, b;
                char comma1, comma2;

                iss >> prefix >> r >> comma1 >> g >> comma2 >> b;

                if (prefix != "color" || comma1 != ',' || comma2 != ',') {
                    throw std::runtime_error("Erro ao fazer parsing da cor: " + line);
                }

                color = glm::vec3(r, g, b);
            }

            // Cria a luz e adiciona na lista
            Light light(position, color, shaderID, lightNumber);
            m_lights.push_back(light);
            lightNumber++;

        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao processar luz " << lightNumber << ": " << e.what() << std::endl;
            continue; // pula essa luz e tenta a próxima
        }
    }

    file.close();
    return m_lights;
}

Camera& SceneLoader::loadCamera(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo de câmera: " << filePath << std::endl;
        return m_camera;
    }

    std::string line;

    auto parseVec3Line = [](const std::string& line, const std::string& expectedPrefix) -> glm::vec3 {
        std::istringstream iss(line);
        std::string prefix;
        float x, y, z;
        char comma1, comma2;

        iss >> prefix >> x >> comma1 >> y >> comma2 >> z;

        if (prefix != expectedPrefix || comma1 != ',' || comma2 != ',')
        {
            throw std::runtime_error("Erro ao fazer parsing da linha: " + line);
        }

        return glm::vec3(x, y, z);
        };

    try
    {
        if (std::getline(file, line))
        {
            glm::vec3 pos = parseVec3Line(line, "position");
            m_camera.setPosition(pos);
        }

        if (std::getline(file, line))
        {
            glm::vec3 look = parseVec3Line(line, "lookAt");
            m_camera.setLookAt(look);
        }

        float fov = 45.0f, aspect = 1.33f, nearPlane = 0.1f, farPlane = 100.0f;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key;

            if (key == "fov")      iss >> fov;
            else if (key == "aspect")  iss >> aspect;
            else if (key == "near")    iss >> nearPlane;
            else if (key == "far")     iss >> farPlane;
        }

        m_camera.setFrustum(fov, aspect, nearPlane, farPlane);
        m_camera.initCamera(shaderID);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erro ao processar câmera: " << e.what() << std::endl;
    }

    file.close();
    return m_camera;
}

