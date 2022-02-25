#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};
    
    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        
        //keyboard movement
        void keyboardCallback(GLFWwindow* window, float deltaTime, float defaultCameraSpeed);

        void switchFPSMode(GLfloat height = 0.0f);

        void printPos();

        glm::vec3 cameraPosition;
        
        bool detectedCollision = false;
        glm::vec3 cameraPrevPos;
        glm::vec3 cameraTarget;

    private:
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        glm::vec3 cameraUp;
        
        GLfloat cameraHeight;
        bool FPSCamera = false;



    };
    
}

#endif /* Camera_hpp */
