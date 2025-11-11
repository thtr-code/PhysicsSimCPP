#include <glad/glad.h> // MUST include glad BEFORE glfw
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cmath>

#include "Camera.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Planet.h"
#include "Grid.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, Camera& camera, float deltaTime);

bool firstMouse = true;
int width = 800;
int height = 600;
float lastX = width / 2.0f;
float lastY = height / 2.0f;

// Use one gravity constant everywhere for the planets
const float GLOBAL_G = 1.0f;

//  n-body integrator using Newtonian gravity
void stepNBody(std::vector<Planet>& planets, float deltaTime) {
    const float soft = 0.2f; // softening term to avoid huge forces at very small distances

    if (planets.empty())
        return;

    std::vector<glm::vec3> acc(planets.size(), glm::vec3(0.0f));

    // Pairwise gravity
    for (size_t i = 0; i < planets.size(); ++i) {
        for (size_t j = i + 1; j < planets.size(); ++j) {
            glm::vec3 dir = planets[j].worldPosition - planets[i].worldPosition;
            float dist2 = glm::dot(dir, dir) + soft * soft;
            float dist = std::sqrt(dist2);

            if (dist <= 0.0f)
                continue;

            glm::vec3 dirNorm = dir / dist;

            float forceMag = GLOBAL_G * planets[i].mass * planets[j].mass / dist2;

            glm::vec3 a_i = dirNorm * (forceMag / planets[i].mass);
            glm::vec3 a_j = -dirNorm * (forceMag / planets[j].mass);

            acc[i] += a_i;
            acc[j] += a_j;
        }
    }

    // Integrate velocities and positions (semi-implicit Euler)
    for (size_t i = 0; i < planets.size(); ++i) {
        planets[i].velocity      += acc[i] * deltaTime;
        planets[i].worldPosition += planets[i].velocity * deltaTime;
    }
}

int main(){
    if(!glfwInit()){
        std::cerr << "Failed to initialize program" << "\n";
        return -1;
    }
    //*****MUST***** HAVE on macOS. Wasted two hours debugging this!! >:(
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //

    GLFWwindow *window =  glfwCreateWindow(width, height, "Title", nullptr, nullptr);
    if(window == NULL){
        std::cerr << "Failed to create window" << "\n";
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { //MUST init after setting current context
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Test triangle (you can ignore this visually)
    float vertices[] = {
            // positions                        // colors
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
    };

    glfwSetCursorPosCallback(window, mouse_callback);

    Shader shader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Define camera
    Camera camera(
            glm::vec3(-17.0f, 4.0f, -8.0f), // pos
            glm::vec3(0.0f, 1.0f, 0.0f),    // WorldUp
            20.0f,                          // yaw
            -20.0f                          // pitch
    );

    // Let mouse callback find the camera
    glfwSetWindowUserPointer(window, &camera);

    // Define shaders
    Shader planetShader("shaders/pvShader.glsl", "shaders/pfShader.glsl");
    Shader gridShader("shaders/gridvert.glsl", "shaders/gridfrag.glsl");

    Grid grid(50, 0.4f);

    std::vector<Planet> planets;

    // Sun-like body (1.0 in this sim's scale)
    float sunRadius = 1.0f;
    float sunMass   = 10.0f;
    planets.emplace_back(
            sunRadius,              // radius
            sunMass,                // mass
            0.0f,                   // orbitAngle
            0.0f,                   // distance
            0.0f,                   // orbitSpeed (sun sits at origin initially)
            0.3f,                   // rotationSpeed
            glm::vec3(1.0f, 0.9f, 0.6f)
    );

    float earthRadius   = 0.1f;      // 10x smaller than sun visually
    float earthDistance = 6.0f;      // distance from origin (sun)
    // Choose orbitSpeed ≈ sqrt(G*M / r) for near-circular orbit
    float earthOrbitSpeed = std::sqrt(GLOBAL_G * sunMass / earthDistance); // ~sqrt(10/6)

    planets.emplace_back(
            earthRadius,
            1.0f,                   // smaller mass
            0.0f,                   // starting angle
            earthDistance,          // distance from origin (sun)
            earthOrbitSpeed,        // orbitSpeed tuned to gravity
            2.0f,                   // rotationSpeed
            glm::vec3(0.3f, 0.4f, 1.0f)
    );

    while(!glfwWindowShouldClose(window)){

        glClearColor(0.0f, 0.0f, 0.0f,0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, camera, deltaTime);

        stepNBody(planets, deltaTime);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f),
                                      800.0f / 600.0f,
                                      0.1f,
                                      100.0f);

        for (auto &p : planets) {
            p.update(currentFrame);
        }

        // Grid gravity sources from all planets
        std::vector<Grid::GravitySource> sources;
        sources.reserve(planets.size());
        for (auto &p : planets) {
            sources.push_back({ p.worldPosition, p.mass });
        }
        grid.update(sources);


        //Planets
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        for (auto &p : planets) {
            p.draw(planetShader);
        }


        //Grid
        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", glm::mat4(1.0f));
        grid.draw(gridShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.processKeyboard(LOOKLEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.processKeyboard(LOOKRIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera->processMouseMovement(xoffset, yoffset);
}
