#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Planet {
public:
    Planet(float radius,
           float mass,
           float orbitAngle,
           float distance,
           float orbitSpeed,
           float rotationSpeed,
           glm::vec3 color);

    void update(float time);
    void draw(Shader &shader);

    void generateSphere(int segments, int rings);

    float radius;
    float mass;
    float orbitAngle;
    float distance;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 color;

    glm::mat4 model;

    glm::vec3 worldPosition;
    glm::vec3 velocity;

    unsigned int VAO, VBO;
    std::vector<float> vertices;
    int vertexCount;
};
