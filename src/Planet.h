#pragma once

#include "Shader.h"

class Planet{
public:
    Planet(float radius, float mass, float orbitAngle, float distance, float orbitSpeed, float rotationSpeed, glm::vec3 color);

    void update(float time);
    void draw(Shader& shader);
    glm::vec3 worldPosition;
    float mass;

private:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    int vertexCount;
    float radius;
    float orbitAngle;
    float distance;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 color;
    glm::mat4 model;
    void generateSphere(int segments, int rings);

};
