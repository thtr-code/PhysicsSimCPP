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

const float GLOBAL_G = 0.9f;

// O(N^2) pairwise gravity with small perf tweaks (fewer sqrt/divs)
void stepNBody(std::vector<Planet>& planets, float deltaTime) {
    const float soft = 0.2f;
    if (planets.empty()) return;

    std::vector<glm::vec3> acc(planets.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < planets.size(); ++i) {
        for (size_t j = i + 1; j < planets.size(); ++j) {
            glm::vec3 d = planets[j].worldPosition - planets[i].worldPosition;
            float dist2 = glm::dot(d, d) + soft * soft;
            float invDist = 1.0f / std::sqrt(dist2);
            glm::vec3 dir = d * invDist;

            // a_i = G * m_j / r^2 in direction dir
            float s = GLOBAL_G / dist2;
            glm::vec3 a_i = dir * (s * planets[j].mass);
            glm::vec3 a_j = -dir * (s * planets[i].mass);

            acc[i] += a_i;
            acc[j] += a_j;
        }
    }

    // Semi-implicit Euler
    for (size_t i = 0; i < planets.size(); ++i) {
        planets[i].velocity += acc[i] * deltaTime;
        planets[i].worldPosition += planets[i].velocity * deltaTime;
    }
}

// Keep center of mass at origin and remove bulk drift velocity.
void enforceCenterOfMassFrame(std::vector<Planet>& planets) {
    if (planets.empty()) return;
    float totalMass = 0.0f;
    glm::vec3 comPos(0.0f), comVel(0.0f);

    for (const auto &p : planets) {
        totalMass += p.mass;
        comPos += p.mass * p.worldPosition;
        comVel += p.mass * p.velocity;
    }
    if (totalMass <= 0.0f) return;
    comPos /= totalMass;
    comVel /= totalMass;

    for (auto &p : planets) {
        p.worldPosition -= comPos;
        p.velocity -= comVel;
    }
}

int main(){
    if(!glfwInit()){
        std::cerr << "Failed to initialize program\n";
        return -1;
    }
    ////*****MUST***** HAVE on macOS. Wasted two hours debugging >:(
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Title", nullptr, nullptr);
    if(window == NULL){
        std::cerr << "Failed to create window\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetCursorPosCallback(window, mouse_callback);

    // Shaders
    Shader planetShader("shaders/pvShader.glsl", "shaders/pfShader.glsl");
    Shader gridShader("shaders/gridvert.glsl", "shaders/gridfrag.glsl");

    // Camera
    Camera camera(
            glm::vec3(-17.0f, 4.0f, -8.0f), // pos
            glm::vec3(0.0f, 1.0f, 0.0f),    // WorldUp
            20.0f,                          // yaw
            -20.0f                          // pitch
    );
    glfwSetWindowUserPointer(window, &camera);

    Grid grid(50, 0.4f);

    std::vector<Planet> planets;

    // Star (Sun)
    float sunRadius = 1.0f;
    float sunMass   = 50.0f;
    planets.emplace_back(
            sunRadius,
            sunMass,
            0.0f,      // orbitAngle
            0.0f,      // distance
            0.0f,      // orbitSpeed
            0.3f,      // rotationSpeed
            glm::vec3(1.0f, 0.9f, 0.6f),
            BodyType::Star
    );

    // Earth
    float earthRadius = 0.08f;
    float earthMass = 1.0f;
    float earthDistance = 4.0f;
    float earthOrbitSpeed = std::sqrt(GLOBAL_G * sunMass / earthDistance);
    planets.emplace_back(
            earthRadius, earthMass,
            0.0f, earthDistance, earthOrbitSpeed,
            2.0f, glm::vec3(0.3f, 0.4f, 1.0f),
            BodyType::Planetary
    );

    // Jupiter-ish
    float jupiterRadius = 0.18f;
    float jupiterMass = 3.0f;
    float jupiterDistance = 11.0f;
    float jupiterOrbitSpeed = std::sqrt(GLOBAL_G * sunMass / jupiterDistance);
    planets.emplace_back(
            jupiterRadius, jupiterMass,
            0.7f, jupiterDistance, jupiterOrbitSpeed,
            1.5f, glm::vec3(0.9f, 0.6f, 0.2f),
            BodyType::Planetary
    );

    enforceCenterOfMassFrame(planets);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while(!glfwWindowShouldClose(window)){
        glClearColor(0.0f, 0.0f, 0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, camera, deltaTime);

        // Physics
        stepNBody(planets, deltaTime);
        enforceCenterOfMassFrame(planets);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                static_cast<float>(width)/height,
                                                0.1f, 100.0f);

        for (auto &p : planets) {
            p.update(currentFrame);
        }

        // Grid sources from all planets
        std::vector<Grid::GravitySource> sources;
        sources.reserve(planets.size());
        for (auto &p : planets) sources.push_back({ p.worldPosition, p.mass });
        grid.update(sources);

        // Draw planets
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);

        for (auto &p : planets) {
            bool isSun = p.isStar();
            glm::vec3 color = isSun ? glm::vec3(1.0f, 0.9f, 0.6f) : p.color;
            planetShader.setVec3("baseColor", color);
            planetShader.setFloat("isSun", isSun ? 1.0f : 0.0f);

            float baseSize = isSun ? 80.0f : 28.0f; // purely visual
            float pointSize = baseSize * p.radius / sunRadius;
            planetShader.setFloat("pointSize", pointSize);

            p.draw(planetShader);
        }

        // Draw grid
        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", glm::mat4(1.0f));
        gridShader.setVec3("gridColor", glm::vec3(0.8f, 0.8f, 0.8f));
        grid.draw(gridShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)     camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)     camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)     camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)     camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.processKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.processKeyboard(LOOKLEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.processKeyboard(LOOKRIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
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
