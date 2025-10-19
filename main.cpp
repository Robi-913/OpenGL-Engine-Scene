//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright   2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC

#include <GL/glew.h>

#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "Skybox.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow *glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint pointLightPosLoc;
GLuint pointLightRender;
glm::vec3 pointLightPos;


gps::Camera myCamera(
        glm::vec3(0.0f, 2.0f, 5.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
GLfloat lightAngle;

enum MovementState {
    LIFT_VACA,
    MOVE_LEFT
};

gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scene;
gps::Model3D vaca;
glm::vec3 vacaInitialPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 vacaCurrentPos = vacaInitialPos;
const float vacaMaxHeight = 15.0f;
bool liftVaca = false;

gps::Model3D ufo;
MovementState currentMovementState = LIFT_VACA;
glm::vec3 ufoInitialPos = glm::vec3(0.0f, 10.0f, 0.0f);
glm::vec3 ufoCurrentPos = ufoInitialPos;
glm::vec3 sharedMoveDirection = glm::vec3(-0.05f, 0.0f, 0.0f);


gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;
gps::Shader rainShader;
gps::Shader pointLightShader;

bool enableRaining = false;
std::vector<glm::vec3> rainingPositions;
const int nbOfPicuri = 6000;

bool pointLightEnabled =  false;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox myNightSkyBox;
gps::SkyBox myLightSkyBox;

bool animateCameraBool = false;
float animationTimer = 0.0f;
const float animationDuration = 12.0f;

glm::vec3 initialCameraPosition = glm::vec3(10.0f, 2.0f, 5.5f);
glm::vec3 topCameraPosition = glm::vec3(0.0f, 30.0f, 5.5f);
float orbitRadius = 27.0f;
glm::vec3 orbitCenter = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 finalCameraPosition = glm::vec3(0.0f, 5.0f, 5.5f);

enum RenderMode {
    SOLID, WIREFRAME, FLAT, SMOOTH
};
RenderMode currentMode = SOLID;

bool showDepthMap;

GLenum glCheckError_(const char *file, int line) {
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

void windowResizeCallback(GLFWwindow *window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    //TODO
}

float yaw = -90.0f;
float pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = 800.0f / 2.0;
    static float lastY = 600.0f / 2.0;

    if (firstMouse) {
        firstMouse = false;
        lastX = xpos;
        lastY = ypos;
    }

    float x_offset = xpos - lastX;
    float y_offset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        liftVaca = !liftVaca; // Toggle lift state
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        pointLightEnabled = !pointLightEnabled;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        enableRaining = !enableRaining; // Toggle rain
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        animateCameraBool = !animateCameraBool;
        animationTimer = 0.0f;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        glm::vec3 cameraPosition = myCamera.getCameraPosition();
        std::cout << "Camera Position: x = " << cameraPosition.x
                  << ", y = " << cameraPosition.y
                  << ", z = " << cameraPosition.z << std::endl;
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

void animationForTheCamera(float deltaTime) {
    if (!animateCameraBool) return;

    animationTimer += deltaTime;

    auto easeInOutCubic = [](float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    };
    // time of the animation
    float lateralOffset = 15.0f;

    if (animationTimer < animationDuration / 3.0f) {
        float t = animationTimer / (animationDuration / 3.0f);
        t = easeInOutCubic(t);
        //moving  the camera up
        glm::vec3 diagonalTarget = glm::vec3(
                orbitCenter.x + lateralOffset,
                topCameraPosition.y,
                orbitCenter.z + orbitRadius
        );
        glm::vec3 newPos = glm::mix(initialCameraPosition, diagonalTarget, t);

        myCamera.setCameraPosition(newPos);
        myCamera.setCameraTarget(orbitCenter);
    } else if (animationTimer < 2.0f * animationDuration / 3.0f) {
        //rotating the camera
        float t = (animationTimer - animationDuration / 3.0f) / (animationDuration / 3.0f);
        t = easeInOutCubic(t);
        float angle = glm::radians(360.0f) * t;

        //orbiting point
        float x = orbitCenter.x + lateralOffset + orbitRadius * 1.4f * cos(angle);
        float z = orbitCenter.z + orbitRadius * 1.4f * sin(angle);
        glm::vec3 newPos = glm::vec3(x, topCameraPosition.y, z);

        myCamera.setCameraPosition(newPos);
        myCamera.setCameraTarget(orbitCenter);
    } else if (animationTimer < animationDuration) {
        //after the rotating from current position we go back to the init state
        float t = (animationTimer - 2.0f * animationDuration / 3.0f) / (animationDuration / 3.0f);
        t = easeInOutCubic(t);

        glm::vec3 rotationEndPosition = glm::vec3(
                orbitCenter.x + lateralOffset + orbitRadius * 1.4f * cos(glm::radians(360.0f)),
                topCameraPosition.y,
                orbitCenter.z + orbitRadius * 1.4f * sin(glm::radians(360.0f))
        );

        glm::vec3 newPos = glm::mix(rotationEndPosition, finalCameraPosition, t);

        myCamera.setCameraPosition(newPos);
        myCamera.setCameraTarget(orbitCenter);
    } else {
        //reset animation
        animateCameraBool = false;
        animationTimer = 0.0f;
    }
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 0.9f;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 0.9f;
    }

    std::vector<gps::BoundingBox> obstacles;

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed, obstacles);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed, obstacles);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed, obstacles);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed, obstacles);
    }

    if (liftVaca) {
        if (currentMovementState == LIFT_VACA) {
            if (vacaCurrentPos.y < vacaMaxHeight) {
                vacaCurrentPos.y += 0.05f;
                pointLightPos.y = vacaCurrentPos.y;
            } else {
                currentMovementState = MOVE_LEFT;
            }
        } else if (currentMovementState == MOVE_LEFT) {
            vacaCurrentPos += sharedMoveDirection;
            ufoCurrentPos += sharedMoveDirection;
            pointLightPos += sharedMoveDirection;
        }
    } else {
        vacaCurrentPos = vacaInitialPos;
        ufoCurrentPos = ufoInitialPos;
        pointLightPos = glm::vec3(-2.58606f, 13.8976f, 15.9664f);
        currentMovementState = LIFT_VACA;
    }

}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentMode = SOLID;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentMode = WIREFRAME;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentMode = FLAT;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentMode = SMOOTH;
}

void setRenderMode() {
    switch (currentMode) {
        case SOLID:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case WIREFRAME:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case FLAT:
            glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
            break;
        case SMOOTH:
            glPolygonMode(GL_FRONT_AND_BACK, GL_SMOOTH);
            break;
    }
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);

    glfwSwapInterval(1);

#if not defined (__APPLE__)
    //start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    //get version info
    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);//cull face
    glCullFace(GL_BACK); //cull back face
    glFrontFace(GL_CCW); //GL_CCW for counter clock-wise

    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
    //all the objects i use in my scene
    lightCube.LoadModel("objects/cube/cube.obj");
    screenQuad.LoadModel("objects/quad/quad.obj");
    scene.LoadModel("scene/finalscene.obj", "scene/");
    ufo.LoadModel("scene/ozn.obj", "scene/");
    vaca.LoadModel("scene/vaca.obj", "scene/");
}

void initShaders() {

    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();

    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();

    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();

    depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
    depthMapShader.useShaderProgram();

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    rainShader.loadShader("shaders/raining.vert", "shaders/raining.frag");
    rainShader.useShaderProgram();

    pointLightShader.loadShader("shaders/pointLight.vert","shaders/pointLight.frag");
    pointLightShader.useShaderProgram();

}


void initUniforms() {
    myCustomShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f), (float) retina_width / (float) retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //setting the direction and the color of the light
    lightDir = glm::vec3(4.0f, 7.0f, 1.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // Lumină albă
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //set pointLight.vert position
    pointLightPos = glm::vec3(-2.2f, 17.0f, 16.0f);
    glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(pointLightPos, 1.0f));
    pointLightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(lightPosEye));

    pointLightRender= glGetUniformLocation(myCustomShader.shaderProgram, "pointLightEnabled");
    glUniform1i(pointLightRender, pointLightEnabled ? 1 : 0);

    //param for the fog
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "fogColor"), 0.5f, 0.5f, 0.5f); //color
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogStart"), 50.0f); //where its starts
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogEnd"), 130.0f);  //the scene is full opaque
}

//init for texture and depth
void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initRain() {
    rainingPositions.resize(nbOfPicuri);
    //get camera position
    glm::vec3 cameraPos = myCamera.getCameraPosition();

    for (int i = 0; i < nbOfPicuri; ++i) {
        float x = cameraPos.x + static_cast<float>(rand() % 200 - 100) / 10.0f;
        float y = cameraPos.y + static_cast<float>(rand() % 100) / 10.0f + 10.0f;
        float z = cameraPos.z + static_cast<float>(rand() % 200 - 100) / 10.0f;
        rainingPositions[i] = glm::vec3(x, y, z);
    }
}

void initNightSkybox() {
    std::vector<const GLchar *> faces;
    faces.push_back("skybox/px.jpg");
    faces.push_back("skybox/nx.jpg");
    faces.push_back("skybox/py.jpg");
    faces.push_back("skybox/ny.jpg");
    faces.push_back("skybox/pz.jpg");
    faces.push_back("skybox/nz.jpg");
    myNightSkyBox.Load(faces);
}


void initLightSkybox() {
    std::vector<const GLchar *> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    myLightSkyBox.Load(faces);
}

void updateRain() {
    glm::vec3 cameraPos = myCamera.getCameraPosition(); // Get the current camera position

    for (int i = 0; i < nbOfPicuri; ++i) {
        rainingPositions[i].y -= 0.2f; //for moving down animation
        if (rainingPositions[i].y < cameraPos.y - 5.0f) { //reseting if its to low
            //this is for random position on the camera
            rainingPositions[i].y = cameraPos.y + static_cast<float>(rand() % 100) / 10.0f + 10.0f;
            rainingPositions[i].x = cameraPos.x + static_cast<float>(rand() % 200 - 100) / 10.0f;
            rainingPositions[i].z = cameraPos.z + static_cast<float>(rand() % 200 - 100) / 10.0f;
        }
    }
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::vec3 lightPos = glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)); //light position
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //look at the scene center
    float nearPlane = 0.1f, farPlane = 100.0f; //adjust near and far planes
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, nearPlane, farPlane); //adjust ortho bounds
    return lightProjection * lightView;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    glm::mat4 sceneModel = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel));
    scene.Draw(shader);

    glm::mat4 vacaModel = glm::mat4(1.0f);
    vacaModel = glm::translate(vacaModel, vacaCurrentPos); // Aplică întâi mișcarea verticală
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(vacaModel));
    vaca.Draw(shader);

    glm::mat4 ufoModel = glm::mat4(1.0f);
    ufoModel = glm::translate(ufoModel, ufoCurrentPos); // Aplică întâi translația specifică
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(ufoModel));
    ufo.Draw(shader);

    //logistic for night and day skybox
    if (!depthPass) {
        if(pointLightEnabled){
            myNightSkyBox.Draw(skyboxShader, view, projection);
        } else{
            myLightSkyBox.Draw(skyboxShader, view, projection);
        }
    }
}

void renderRain(gps::Shader shader) {
    shader.useShaderProgram();

    GLuint modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    GLuint lightDirLoc = glGetUniformLocation(shader.shaderProgram, "lightDir");
    GLuint timeLoc = glGetUniformLocation(shader.shaderProgram, "time");

    // Pass view and projection matrices
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //passing the light direction for more realistic rain calculation
    glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, 0.5f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //this code is used for make the flickering more realistic
    float currentTime = static_cast<float>(glfwGetTime());
    glUniform1f(timeLoc, currentTime);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec3 cameraPos = myCamera.getCameraPosition();
    glm::vec3 cameraFront = glm::normalize(myCamera.getCameraFront());
    glm::vec3 cameraUp = myCamera.getCameraUp();
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

    for (int i = 0; i < nbOfPicuri; ++i) {
        glm::vec3 raindropPos = rainingPositions[i];
        float windEffect = static_cast<float>(sin(glfwGetTime() * 0.5)) * 0.05f;
        raindropPos.x += windEffect;

        glm::vec3 scale = glm::vec3(0.02f, 0.15f, 0.02f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, raindropPos);
        model[0] = glm::vec4(cameraRight, 0.0f);
        model[1] = glm::vec4(cameraUp, 0.0f);
        model[2] = glm::vec4(-cameraFront, 0.0f);
        model = glm::scale(model, scale);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        screenQuad.Draw(shader);
    }

    glDisable(GL_BLEND);
}

void renderScene() {
    //depthmap randering
    depthMapShader.useShaderProgram();

    //calculate the light space
    glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    //viewpoint for shadows
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //render scene to depthmap
    drawObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //main randering
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myCustomShader.useShaderProgram();

    //update the view after moving the camera
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform1i(pointLightRender, pointLightEnabled ? 1 : 0);

    //updata light direction by rotateing
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 lightDirTransformed = glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir;
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTransformed));

    //passing the light matrix
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    //binding the textures to the map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

    //render the shadows
    drawObjects(myCustomShader, false);

    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }

    if (enableRaining) {
        updateRain();
        renderRain(rainShader);
    }

    if (pointLightEnabled) {
        myCustomShader.useShaderProgram();

        glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(pointLightPos, 1.0f));
        glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(lightPosEye));

        //this cod is used to brow the cube so i can know where the light point is
        pointLightShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(pointLightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(pointLightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPos);
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        glUniformMatrix4fv(glGetUniformLocation(pointLightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    }

}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

int main(int argc, const char *argv[]) {

    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms();
    initFBO();
    initRain();

    initNightSkybox();
    initLightSkybox();

    glCheckError();

    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(glWindow)) {
        //frame time calculation
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        processInput(glWindow);
        setRenderMode();

        processMovement();
        renderScene();

        if (animateCameraBool) {
            animationForTheCamera(deltaTime);
        }

        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }

    cleanup();

    return 0;
}
