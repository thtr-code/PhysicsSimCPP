#include "Grid.h"
#include <cmath>

Grid::Grid(int gridcount, float gridspacing):
        gridcount(gridcount), gridspacing(gridspacing)
{
    generateGrid();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data() , GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    vertexCount = vertices.size() / 3;

}


void Grid::generateGrid(){
    vertices.clear();
    vertices.reserve(gridcount * gridcount * 3);

    for (int z = 0; z < gridcount; z++) {
        for (int x = 0; x < gridcount; x++) {

            float cx = x - (gridcount / 2.0f);
            float cz = z - (gridcount / 2.0f);

            float worldX = cx * gridspacing;
            float worldZ = cz * gridspacing;
            float worldY = 0.0f;

            vertices.push_back(worldX);
            vertices.push_back(worldY);
            vertices.push_back(worldZ);
        }
    }
}

void Grid::update(const std::vector<Grid::GravitySource> &sources) {

    for (int i = 0; i < vertexCount; i++) {

        float &x = vertices[i*3 + 0];
        float &y = vertices[i*3 + 1];
        float &z = vertices[i*3 + 2];

        float dip = 0.0f;

        // Make the grid dip weaker so it stays in view
        float G = 0.1f;      // was 1.0f before
        float soft = 0.5f;

        for (const auto &src : sources) {

            float dx = x - src.position.x;
            float dz = z - src.position.z;
            float dy = 0.0f - src.position.y;

            float dist = std::sqrt(dx*dx + dy*dy + dz*dz + soft*soft);

            dip += -G * src.mass / dist;  // U ~ -G * m / r
        }

        y = dip;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    vertices.size() * sizeof(float),
                    vertices.data());
}

void Grid::draw(Shader &shader)
{
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int N = gridcount;

    for (int z = 0; z < N; z++) {
        int start = z * N;
        glDrawArrays(GL_LINE_STRIP, start, N);
    }

    for (int x = 0; x < N; x++) {
        std::vector<GLuint> columnIndices;
        columnIndices.reserve(N);

        for (int z = 0; z < N; z++) {
            columnIndices.push_back(z * N + x);
        }

        GLuint tempEBO;
        glGenBuffers(1, &tempEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     columnIndices.size() * sizeof(GLuint),
                     columnIndices.data(),
                     GL_STATIC_DRAW);

        glDrawElements(GL_LINE_STRIP, N, GL_UNSIGNED_INT, 0);

        glDeleteBuffers(1, &tempEBO);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
