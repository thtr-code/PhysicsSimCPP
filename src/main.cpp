#include <glad/glad.h> // MUST include glad BEFORE glfw
#include <GLFW/glfw3.h>
#include <iostream>
#include "Camera.h"
#include "Shader.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Planet.h"
#include "Grid.h"

void processInput(GLFWwindow* window, Camera& camera, float deltaTime);


int width = 800;
int height = 600;

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

    float vertices[] = {
            // positions                        // colors
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
    };

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


    //Define camera(s)
    Camera camera(
            glm::vec3(-17.0f, 4.0f, -8.0f), //pos
            glm::vec3(0.0f, 1.0f, 0.0f), //WorldUp
            20.0f, //yaw
            -20.0f //pitch
            );
    //Define shaders
    Planet earth(1, 1.0f,  M_PI, 5, 1, 2, glm::vec3(0.3f, 0.2f, 0.2f));
    Shader planetShader("shaders/pvShader.glsl", "shaders/pfShader.glsl");

    Shader gridShader("shaders/gridvert.glsl", "shaders/gridfrag.glsl");
    Grid grid(200, 0.1f);

    while(!glfwWindowShouldClose(window)){

        glClearColor(0.0f, 0.0f, 0.0f,0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Frame calculation
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, camera, deltaTime);


        //Define projection and view for every shader s.t they are affected by the camera view
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        planetShader.use();

        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        earth.draw(planetShader);

        std::vector<Grid::GravitySource> sources;
        sources.push_back({ earth.worldPosition, earth.mass });

        grid.update(sources);
        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", glm::mat4(1.0f));
        grid.draw(gridShader);
        earth.update(glfwGetTime());

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
}