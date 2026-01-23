#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Material {
    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct Light {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

// A small subset of the table (Emerald, Gold, Ruby, etc.)
const std::vector<Material> materials = {
    { "Emerald",    {0.0215f, 0.1745f, 0.0215f}, {0.07568f, 0.61424f, 0.07568f}, {0.633f, 0.727811f, 0.633f}, 0.6f * 128.0f },
    { "Gold",       {0.24725f, 0.1995f, 0.0745f}, {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 0.4f * 128.0f },
    { "Ruby",       {0.1745f, 0.01175f, 0.01175f}, {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 0.6f * 128.0f },
    { "Chrome",     {0.25f, 0.25f, 0.25f},       {0.4f, 0.4f, 0.4f},             {0.774597f, 0.774597f, 0.774597f}, 0.6f * 128.0f },
    { "Silver",     {0.19225f, 0.19225f, 0.19225f}, {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 0.4f * 128.0f },
    { "Cyan Plastic", {0.0f, 0.1f, 0.06f},      {0.0f, 0.50980392f, 0.50980392f}, {0.50196078f, 0.50196078f, 0.50196078f}, 0.25f * 128.0f }
};