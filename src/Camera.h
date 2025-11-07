#pragma once
#include <glm/gtc/type_ptr.hpp>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};
class Camera{

public:
    glm::vec3 position, worldUp;
    glm::vec3 right, up, front;
    float yaw, pitch;
    float speed;
    float sensitivity;
    float zoom;
    Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch);
    glm::mat4 getViewMatrix();
    void processKeyboard(Camera_Movement direction, float deltaTime);
private:
   void updateCameraVectors();

};

