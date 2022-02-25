#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>

#pragma comment(lib, "irrKlang.lib")
#include "irrKlang/include/irrKlang.h"
// window
gps::Window myWindow;
int glWindowHeight = 1920;
int glWindowWidth = 1200;
// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 pointLightDir;
glm::vec3 pointLightColor;


// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

GLint pointLightDirLoc;
GLint pointLightColorLoc;

// camera
GLfloat camera_height = 3.0f;
gps::Camera myCamera(
    glm::vec3(-71.0f, camera_height, -428.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));


GLfloat defaultCameraSpeed = 0.1f;
GLfloat defaultMouseSensitivity = 2.0f;

irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

//control
GLboolean pressedKeys[1024];
bool showDepthMap = false;
bool toggleSnow = false;
bool collisionEnabled = false;
bool rec = false;
bool playRec = false;
bool enableHDR = true;
bool enableSound = false;
std::ifstream fin;
std::ofstream fout;
// models
std::vector<std::shared_ptr<gps::Model3D> > models;
gps::Model3D screenQuad;
gps::Model3D lightCube;

GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader screenQuadShader;
gps::Shader lightShader;
gps::Shader skyboxShader;
gps::Shader hdrShader;
gps::Shader blurShader;
gps::Shader geometryShader;
//shadows
glm::mat4 lightRotation;
GLfloat lightAngle;


const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;
GLuint shadowMapFBO;
GLuint depthMapTexture;


//camera movement
float deltaTime = 0.0f;
float lastFrame = glfwGetTime();

//skybox
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;


void updateDelta()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

float lastX = glWindowWidth / 2;
float lastY = glWindowHeight / 2;
float cameraYaw = 0;
float cameraPitch = 0;
bool firstMouse = true;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwSetWindowSize(window, width, height);
    projection = glm::perspective(glm::radians(90.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 100000.0f);
}
void record()
{
    
    fout << myCamera.cameraPosition.x << " " << myCamera.cameraPosition.y << " " << myCamera.cameraPosition.z << std::endl;
}
std::vector<glm::vec3> camRecording;

void readRec()
{
    fin.open("rec.txt");
    camRecording.clear();
    float x, y, z;
    fin.seekg(0, fin.beg);
    while (fin >> x >> y >> z){
        camRecording.emplace_back(x, y, z);
    }
    
   
}
void playRecording()
{
    static int step = 0;
    if (camRecording.size())
    {
        myCamera.cameraTarget += camRecording[step % camRecording.size()] - myCamera.cameraPosition;
        myCamera.cameraPosition = camRecording[step % camRecording.size()];
        step++;
    }
}

void playMusic(bool sound) {

    if (!engine)
        return;
    engine->play2D("irythrill.ogg", false);
    if (!sound)
        engine->stopAllSounds();
}
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        myCamera.switchFPSMode(camera_height);
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        enableHDR = !enableHDR;
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {

        playMusic(enableSound = !enableSound);
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        if (!rec)
        {
            fout.open("rec.txt");

        }
        else
        {
            fout.close();
        }
        rec = !rec;

    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        if (!playRec)
        {
            readRec();
        }
        else
        {
            fin.close();
        }
        std::cout << camRecording.size() << std::endl;
        playRec = !playRec;

    }
	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (key == GLFW_KEY_J && action == GLFW_PRESS) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        enableHDR = true;
    }

    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        enableHDR = false;

    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        enableHDR = false;

    }

    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
       
        toggleSnow = !toggleSnow;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {

        collisionEnabled = !collisionEnabled;
    }

}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
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

    
    xoffset *= defaultMouseSensitivity;
    yoffset *= defaultMouseSensitivity;

    cameraYaw -= glm::radians(xoffset);
    cameraPitch += glm::radians(yoffset);
    if (cameraPitch > 89.0f)
    {
        cameraPitch = 89.0f;
    }
    else if (cameraPitch < -89.0f)
    {
        cameraPitch = -89.0f;
    }
  
}

void processMovement() {
    const float defaultCameraSpeed = 20.0f;
    float cameraSpeed = defaultCameraSpeed * deltaTime;
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT])
    {
        cameraSpeed *= 2.5;
    }
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        if(rec)
            record();

		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        if (rec)
            record();

        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        if (rec)
            record();

        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        if (rec)
            record();

        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 10.0f * deltaTime;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 10.0f * deltaTime;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    
    if (pressedKeys[GLFW_KEY_R]) {
        lightAngle -= 10.0f * deltaTime;
    }

    if (pressedKeys[GLFW_KEY_T]) {
        lightAngle += 10.0f * deltaTime;
    }

    myCamera.rotate(glm::radians(cameraPitch), glm::radians(cameraYaw));
    //update view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void initOpenGLWindow() {
    myWindow.Create(glWindowHeight, glWindowWidth, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"


	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void addModel(const char* path)
{   
    models.emplace_back(new gps::Model3D());
    models.back()->LoadModel(path);
}


void initModels() {
    
    screenQuad.LoadModel("models/quad/quad.obj");
    lightCube.LoadModel("models/cube/cube.obj");


    addModel("models/sceneFull/scene.obj");
    addModel("models/nanosuit/nanosuit.obj");
    addModel("models/dragon/dragon_wings.obj");
    addModel("models/dragon/dragon.obj");
    addModel("models/noCollision/scene.obj");
    addModel("models/anorLondo/anorLondo.obj");



}

void initShaders() {
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	myBasicShader.loadShader("shaders/basic.vert","shaders/basic.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    hdrShader.loadShader("shaders/hdr.vert", "shaders/hdr.frag");
    blurShader.loadShader("shaders/blur.vert", "shaders/blur.frag");
    geometryShader.loadShader("shaders/geometry.vert", "shaders/geometry.frag", "shaders/geometry.geom");

    
                

}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(90.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 100000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
    lightDir = glm::vec3(-45.0f, 18.0f, -63.0f);

    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.5f, 1.5f, 1.5f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));



    //point light 1

    pointLightDir = glm::vec3(-46.0f, 3.76f, -25.18f);
    pointLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightDir");

    // send light dir to shader
    glUniform3fv(pointLightDirLoc, 1, glm::value_ptr(pointLightDir));

    //set light color
    pointLightColor = glm::vec3(3.0f, 0.4f, 0.0f); //yellow
    pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    // send light color to shader
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
    //point light 1
   glUniform3fv( glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos0"), 1, glm::value_ptr(glm::vec3(-46.202702f, 3.607563f, -45.813202f)));
   glUniform3fv( glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor0"), 1, glm::value_ptr(glm::vec3(3.0f, 0.4f, 0.0f)));

   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos1"), 1, glm::value_ptr(glm::vec3(-31.409397, 4.687797, -36.248096)));
   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor1"), 1, glm::value_ptr(glm::vec3(3.0f, 0.4f, 0.0f)));

   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos2"), 1, glm::value_ptr(glm::vec3(-31.151337, 4.360733, -31.291210)));
   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor2"), 1, glm::value_ptr(glm::vec3(3.0f, 0.4f, 0.0f)));

   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos3"), 1, glm::value_ptr(glm::vec3(-102.326508, 6.463372, -2.795858)));
   glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor3"), 1, glm::value_ptr(glm::vec3(3.0f, 0.4f, 0.0f)));
    //light cube projection matrix
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glCheckError();

    hdrShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(hdrShader.shaderProgram, "hdrBuffer"), 0);
    glUniform1i(glGetUniformLocation(hdrShader.shaderProgram, "bloomBlur"), 1);

    blurShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(blurShader.shaderProgram, "image"), 0);

    geometryShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void initFBO() {
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    const GLfloat near_plane = 0.1f, far_plane = 90.0f;
    glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(-61.0f,0.0f,-31.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    const float k = 15;
	glm::mat4 lightProjection = glm::ortho(-10.0f *k, 10.0f*k, -10.0f *k, 10.0f*k, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;

}

std::vector<glm::vec3> minBoundingBoxes;
std::vector<glm::vec3> maxBoundingBoxes;
void computeBoundingBoxes()
{
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));

    for (int j = 0; j < models[0]->meshes.size(); j++)
    {
        glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        glm::vec3 max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
        for (int v = 0; v < models[0]->meshes[j].vertices.size(); v++)
        {


            glm::vec3 pos = models[0]->meshes[j].vertices[v].Position;
            min.x = std::min(min.x, pos.x);
            min.y = std::min(min.y, pos.y);
            min.z = std::min(min.z, pos.z);

            max.x = std::max(max.x, pos.x);
            max.y = std::max(max.y, pos.y);
            max.z = std::max(max.z, pos.z);

        }
        if (distance(min, max) < 90.0f)
        {
            minBoundingBoxes.emplace_back(min);
            maxBoundingBoxes.emplace_back(max);
        }
        
    }

}


bool checkCollision()
{
    for (int i = 0; i < minBoundingBoxes.size(); i++)
    {

        glm::vec3 min = (model * glm::vec4(minBoundingBoxes[i], 1.0f));
        glm::vec3 max = (model * glm::vec4(maxBoundingBoxes[i], 1.0f));
        if (min.x <= myCamera.cameraPosition.x && myCamera.cameraPosition.x <= max.x &&
            min.y <= myCamera.cameraPosition.y && myCamera.cameraPosition.y <= max.y &&
            min.z <= myCamera.cameraPosition.z && myCamera.cameraPosition.z <= max.z)
        {
            myCamera.cameraPosition = myCamera.cameraPrevPos;
            return true;

        }
    }
    return false;
}
float dragonWingsAngleX = 0.0f;
float dragonWingsAngleY = 0.0f;
float direction = 1.0f;

void animateDragonWings()
{
    if (dragonWingsAngleX >=40.0f)
    {
        direction = -1.0f;
    }
    else if(dragonWingsAngleX <= -50.0f)
    {
        direction = 1.0f;

    }
    model = glm::scale(model, glm::vec3(5.0f));
    model = glm::rotate(model, glm::radians(dragonWingsAngleY), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(30.0f, 30.0f, 10.0f));
    model = glm::rotate(model, glm::radians(dragonWingsAngleX), glm::vec3(1, 0, 0));
    dragonWingsAngleX += deltaTime * 30.0f * direction;
    dragonWingsAngleY -= deltaTime * 5.0f ;



}
void animateDragon()
{
    if (dragonWingsAngleX >= 40.0f)
    {
        direction = -1.0f;
    }
    else if (dragonWingsAngleX <= -15.0f)
    {
        direction = 1.0f;

    }
    model = glm::scale(model, glm::vec3(5.0f));
    model = glm::rotate(model, glm::radians(dragonWingsAngleY), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(30.0f, 30.0f, 10.0f));



}
void renderObjects(gps::Shader shader, bool depthPass) {
    // select active shader program


    shader.useShaderProgram();

    for (int i=0; i < models.size(); i++)
    {
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));


        switch (i)
        {
        case 2:
            animateDragonWings();
            break;
        case 3:
            animateDragon();
            break;
        
        }
        
        //send teapot model matrix data to shader
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        //send teapot normal matrix data to shader
        if (!depthPass) {
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }

        // draw scene
        models[i]->Draw(shader);
    }
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));


}

void initSkybox()
{
    faces.push_back("textures/skybox/right.png");
    faces.push_back("textures/skybox/left.png");

    faces.push_back("textures/skybox/top.png");

    faces.push_back("textures/skybox/bottom.png");
    faces.push_back("textures/skybox/front.png");
    faces.push_back("textures/skybox/back.png");

    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

//passes
void drawQuad() {

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);


    glClear(GL_COLOR_BUFFER_BIT);

    screenQuadShader.useShaderProgram();

    //bind the depth map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

    glDisable(GL_DEPTH_TEST);
    screenQuad.Draw(screenQuadShader);
    glEnable(GL_DEPTH_TEST);
}
void depthPass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void lightPass()
{

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    renderObjects(myBasicShader, false);
}

void drawSkybox()
{
    glm::mat4 skyView = glm::rotate(view, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    skyView = glm::rotate(skyView, glm::radians(150.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    skyView = glm::rotate(skyView, -lightAngle/45.0f, glm::vec3(0.0f, 1.0f, 1.0f));

    mySkyBox.Draw(skyboxShader, skyView, projection);
}

void drawLightCube()
{
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    model = lightRotation;
    model = glm::translate(model, 1.0f * lightDir);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    lightCube.Draw(lightShader);

}

void drawLightCube2()
{
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    //model = glm::mat4(1.0f);
    model = glm::translate(model, 1.0f * pointLightDir);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    lightCube.Draw(lightShader);

}

GLuint hdrFBO;
GLuint colorBuffer[2];
GLuint rboDepth;
GLuint pingpongFBO[2];
GLuint pingpongColorbuffers[2];
void initHDR()
{
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // create floating point color buffer
    glGenTextures(2, colorBuffer);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
    }

    // create depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    // attach buffers
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ping-pong-framebuffer for blurring
   
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}




float points[100*5] = {
       -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, // top-left
        0.5f,  0.5f, 1.0f, 1.0f, 1.0f, // top-right
        0.5f, -0.5f, 1.0f, 1.0f, 1.0f, // bottom-right
       -0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // bottom-left
};
unsigned int VBO, VAO;

void genSnowflakes()
{
    srand(time(NULL));
    for (int i = 0; i < 100; i++)
    {
        points[i * 5] = -1.0f +rand()%200/100.0f;
        points[i * 5+1] =-1.0f + rand() % 200 / 100.0f;
        points[i * 5+2] = 1.0f;
        points[i * 5+3] = 1.0f;
        points[i * 5+4] = 1.0f;
    }
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    glPointSize(2);
}


void twoPassGaussianBlurHDR()
{
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    blurShader.useShaderProgram();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glUniform1i(glGetUniformLocation(blurShader.shaderProgram, "horizontal"), horizontal);

        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffer[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
        renderQuad();

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

  
    hdrShader.useShaderProgram();


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
    renderQuad();

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    //glBindVertexArray(VAO);

  

}
void snowfall()
{
    glDisable(GL_DEPTH_TEST); // enable depth-testing

    geometryShader.useShaderProgram();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    //glBindVertexArray(0);
    for (int i = 0; i < 100; i++)
    {
        if (points[i * 5 + 1] < -1.0f)
        {
            points[i * 5 + 1] = 1.0f;
            points[i * 5] = -1.0f + rand() % 200 / 100.0f;
        }
        points[i * 5 + 1] = points[i * 5 + 1] - rand() % 100 / 20000.0f;

    }

    glDrawArrays(GL_POINTS, 0, 99);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void renderScene() {
    updateDelta();
    
    
    depthPass();
    if (showDepthMap)
    {
        drawQuad();
        return;
    }
    if (enableHDR)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (playRec)
    {
        playRecording();
    }
    lightPass();
    drawSkybox();
    if(toggleSnow)
        snowfall();
    if (collisionEnabled)
    {
        checkCollision();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //myCamera.printPos();
    if (enableHDR)
    {
        twoPassGaussianBlurHDR();

    }
    
    
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(0.0, 0.0, 0.0, 1.0);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    //renderObjects(myBasicShader, false);
    //drawLightCube2();
    //drawLightCube();
    //myCamera.printPos();
    //renderNanosuit(myBasicShader, false);

   

}


void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
	initShaders();
	initUniforms();
    initFBO();
    initSkybox();
    setWindowCallbacks();
    initHDR();
    genSnowflakes();
    computeBoundingBoxes();

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();
    return EXIT_SUCCESS;
}
