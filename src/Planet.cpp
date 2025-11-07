#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>

Planet::Planet(float radius, float mass, float orbitAngle, float distance, float orbitSpeed, float rotationSpeed, glm::vec3 color):

radius(radius), mass(mass), orbitAngle(orbitAngle),distance(distance),
orbitSpeed(orbitSpeed), rotationSpeed(rotationSpeed), color(color),
model(glm::mat4(1.0f)) {

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    generateSphere(128, 128);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


} ;

void Planet::update(float time) {
    orbitAngle += orbitSpeed * 0.005f;

    model = glm::mat4(1.0f);

    float x = distance * cos(orbitAngle);
    float z = distance * sin(orbitAngle);

    worldPosition = glm::vec3(x, 0.0f, z);

    model = glm::translate(model, glm::vec3(distance * cos(orbitAngle),
                                            0.0f,
                                            distance * sin(orbitAngle)));

    model = glm::rotate(model, time * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

    model = glm::scale(model, glm::vec3(radius));
}
void Planet::generateSphere(int segments, int rings) {


    for (int i = 0; i<= rings; i++) {
        float theta = (float)i / rings * M_PI;
        for (int j = 0; j <= segments; j++) {
            float phi   = (float)j / segments * 2.0f * M_PI;


            float x = radius * sin(theta) * cos(phi);
            float y = radius * cos(theta);
            float z = radius * sin(theta) * sin(phi);

            float r = (x / radius + 1.0f) / 2.0f;
            float g = (y / radius + 1.0f) / 2.0f;
            float b = (z / radius + 1.0f) / 2.0f;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(r);
            vertices.push_back(g);
            vertices.push_back(b);
        }
    }
    vertexCount = vertices.size() / 6; // 6 floats per vertex (x,y,z,r,g,b)

}
void Planet::draw(Shader &shader) {

    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);


}
