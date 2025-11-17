#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Grid {
public:
    struct GravitySource {
        glm::vec3 position;
        float mass;
    };

    Grid(int gridcount, float gridspacing);

    void update(const std::vector<Grid::GravitySource> &sources);
    void draw(Shader &shader);

    // Set where the grid is centered in world space (x,z). y is always 0.
    void setOrigin(const glm::vec3 &newOrigin);

private:
    void generateGrid();

    int gridcount;
    float gridspacing;
    GLuint VAO, VBO;
    int vertexCount;
    std::vector<float> vertices;

    glm::vec3 origin; // world-space center in XZ for the grid
};
