#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"

class Grid{
public:
    Grid(int gridcount, float gridspacing);
    struct GravitySource {
        glm::vec3 position;
        float mass;
    };
    void update(const std::vector<GravitySource> &sources);
    void draw(Shader &shader);

private:
    int gridcount;
    float gridspacing;

    unsigned int VAO, VBO;
    std::vector<float> vertices;
    int vertexCount;

    void generateGrid();


};