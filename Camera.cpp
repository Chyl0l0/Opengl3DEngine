#include "Camera.hpp"
namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUp = cameraUp;
        
        cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        cameraRightDirection = -glm::normalize(glm::cross( cameraFrontDirection, cameraUp));
        cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        cameraHeight = cameraPosition.y;
        switchFPSMode(cameraHeight);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() { 
        //TODO

        return glm::mat4(glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection));

    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        /*
        if (detectedCollision)
        {
            cameraPosition = cameraPrevPos;
            return;
        }
        */
        cameraPrevPos = cameraPosition;

        switch (direction)
        {
        case gps::MOVE_FORWARD:
            cameraPosition -= speed * cameraFrontDirection;
            break;
        case gps::MOVE_BACKWARD:
            cameraPosition += speed * cameraFrontDirection;
            break;
        case gps::MOVE_RIGHT:
            cameraPosition -= speed * cameraRightDirection;
            break;
        case gps::MOVE_LEFT:
            cameraPosition += speed * cameraRightDirection;  

            break;
        default:
            break;
        }
        if (FPSCamera)
        {
            cameraPosition.y = cameraHeight;

        }
        cameraTarget = cameraPosition - cameraFrontDirection;

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        
        cameraFrontDirection = -glm::normalize( glm::yawPitchRoll(yaw, pitch, 0.0f) * glm::vec4(0, 0, -1, 0));
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
        cameraTarget = cameraPosition - cameraFrontDirection;
        

        
    }
    void Camera::keyboardCallback(GLFWwindow* glWindow, float deltaTime, float defaultCameraSpeed)
    {
        float speed = defaultCameraSpeed * deltaTime;
        if (glfwGetKey(glWindow, GLFW_KEY_W) == GLFW_PRESS) {
            move(gps::MOVE_FORWARD, speed);
        }
        if (glfwGetKey(glWindow, GLFW_KEY_S) == GLFW_PRESS) {
            move(gps::MOVE_BACKWARD, speed);
        }
        if (glfwGetKey(glWindow, GLFW_KEY_A) == GLFW_PRESS) {
            move(gps::MOVE_LEFT, speed);
        }
        if (glfwGetKey(glWindow, GLFW_KEY_D) == GLFW_PRESS) {
            move(gps::MOVE_RIGHT, speed);
        }
    }

    void Camera::switchFPSMode( GLfloat  height )
    {
        FPSCamera = not FPSCamera;
        
        cameraPosition.y = cameraHeight = height;
    }

    void Camera::printPos()
    {
        std::cout << glm::to_string(cameraPosition) << std::endl;

    }



}