#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <glad/glad.h>

Planet::Planet(float radius, float mass, float orbitAngle, float distance,
               float orbitSpeed, float rotationSpeed, glm::vec3 color,
               BodyType type)
        : radius(radius), mass(mass), orbitAngle(orbitAngle), distance(distance),
          orbitSpeed(orbitSpeed), rotationSpeed(rotationSpeed), color(color),
          model(glm::mat4(1.0f)), worldPosition(0.0f), velocity(0.0f),
          bodyType(type)
{
    // Initial position in XZ-plane, y a bit above the grid
    worldPosition = glm::vec3(
            distance * std::cos(orbitAngle),
            1.0f,
            distance * std::sin(orbitAngle)
    );

    // Tangential velocity for approx circular motion
    glm::vec3 tangent = glm::normalize(glm::vec3(-std::sin(orbitAngle), 0.0f, std::cos(orbitAngle)));
    velocity = tangent * orbitSpeed;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    generateSphere(128, 128, color);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Planet::update(float time) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, worldPosition);
    model = glm::rotate(model, time * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(radius));
}

void Planet::generateSphere(int segments, int rings, const glm::vec3 &surfaceColor) {
    vertices.clear();
    for (int i = 0; i <= rings; i++) {
        float theta = (float)i / (float)rings * (float)M_PI;
        for (int j = 0; j <= segments; j++) {
            float phi = (float)j / (float)segments * 2.0f * (float)M_PI;

            float x = std::sin(theta) * std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta) * std::sin(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            vertices.push_back(surfaceColor.r);
            vertices.push_back(surfaceColor.g);
            vertices.push_back(surfaceColor.b);
        }
    }
    vertexCount = (int)(vertices.size() / 6);
}

void Planet::draw(Shader &shader) {
    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
}
