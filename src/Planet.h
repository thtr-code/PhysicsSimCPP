#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"

enum class BodyType { Star, Planetary };

class Planet {
public:
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

    Planet(float radius, float mass, float orbitAngle, float distance,
           float orbitSpeed, float rotationSpeed, glm::vec3 color,
           BodyType type = BodyType::Planetary);

    void update(float time);
    void draw(Shader &shader);
    bool isStar() const { return bodyType == BodyType::Star; }

private:
    void generateSphere(int segments, int rings, const glm::vec3 &surfaceColor);

    BodyType bodyType;

    std::vector<float> vertices;
    int vertexCount = 0;
    unsigned int VAO = 0, VBO = 0;
};
