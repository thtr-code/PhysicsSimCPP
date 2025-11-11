#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Planet::Planet(float radius, float mass, float orbitAngle, float distance,
               float orbitSpeed, float rotationSpeed, glm::vec3 color)
        : radius(radius),
          mass(mass),
          orbitAngle(orbitAngle),
          distance(distance),
          orbitSpeed(orbitSpeed),
          rotationSpeed(rotationSpeed),
          color(color),
          model(glm::mat4(1.0f)),
          worldPosition(0.0f),
          velocity(0.0f) {

    // Initial position in XZ-plane, slightly above the grid (y = 1.0f)
    worldPosition = glm::vec3(
            distance * std::cos(orbitAngle),
            1.0f,
            distance * std::sin(orbitAngle)
    );

    // Initial tangential velocity for approximate circular motion
    // (perpendicular to radius in XZ-plane)
    glm::vec3 tangent = glm::normalize(glm::vec3(
            -std::sin(orbitAngle),
            0.0f,
            std::cos(orbitAngle)
    ));
    velocity = tangent * orbitSpeed;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    generateSphere(128, 128);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Planet::update(float time) {

    model = glm::mat4(1.0f);

    model = glm::translate(model, worldPosition);

    model = glm::rotate(model, time * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

    model = glm::scale(model, glm::vec3(radius));
}

void Planet::generateSphere(int segments, int rings) {
    vertices.clear();

    for (int i = 0; i <= rings; i++) {
        float theta = static_cast<float>(i) / static_cast<float>(rings) * static_cast<float>(M_PI);
        for (int j = 0; j <= segments; j++) {
            float phi = static_cast<float>(j) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);

            // Unit sphere: radius = 1.0
            float x = std::sin(theta) * std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta) * std::sin(phi);

            // Simple color based on position
            float r = (x + 1.0f) * 0.5f;
            float g = (y + 1.0f) * 0.5f;
            float b = (z + 1.0f) * 0.5f;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(r);
            vertices.push_back(g);
            vertices.push_back(b);
        }
    }

    vertexCount = static_cast<int>(vertices.size() / 6); // 6 floats per vertex
}

void Planet::draw(Shader &shader) {
    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
}
