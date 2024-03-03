#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

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

// window
gps::Window myWindow;
gps::Shader skyboxShader;
gps::SkyBox mySkyBox;


// matrices
glm::mat4 modelMatrixB;
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

GLfloat lightAngle;

// camera
//glm::vec3(5.0f, 2.0f, 3.0f) camera pos
//glm::vec3(0.0f, 0.0f, -10.0f), camera look at
gps::Camera myCamera(
    //glm::vec3(5.0f, 2.0f, 3.0f),
    //glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(5.0f, 20.0f, 3.0f), //3 z doar
    glm::vec3(5.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::vec3(10.0f, 2.0f, 27.0f),
    //glm::vec3(25.0f, 0.0f, 25.0f),
    //glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

int startAnimationCount = 0;

GLboolean pressedKeys[1024];
GLboolean boatAnimationForward;
int dirLight = 0;
int spotLight = 1;
glm::vec3 pos;
GLuint lightPosLoc;
int fogOn = 0;
//float rotationAngle;
 
// models
//gps::Model3D teapot;
gps::Model3D scene;
gps::Model3D boat;

GLfloat delta = 0.00f;

GLfloat angle;

// shaders
gps::Shader myBasicShader;

bool startAnimation = false;

GLenum glCheckError_(const char *file, int line)
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
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        startAnimation = false;
    }

    // solid
    if (pressedKeys[GLFW_KEY_O]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        startAnimation = false;
    }

    // wireframe
    if (pressedKeys[GLFW_KEY_L]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        startAnimation = false;
    }

    // polygonal
    if (pressedKeys[GLFW_KEY_P]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        startAnimation = false;
    }

    // boat animation
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        boatAnimationForward = !boatAnimationForward;
        startAnimation = false;
    }

    // camera animation
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        startAnimation =! startAnimation;
    }

    if (pressedKeys[GLFW_KEY_X]) {
        myBasicShader.useShaderProgram();
        //glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "dirLight"), dirLight);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "dirLight"), dirLight);
        dirLight = !dirLight;
    }

    if (pressedKeys[GLFW_KEY_F]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogOn"), fogOn);
        fogOn = !fogOn;
    }
    

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    startAnimation = false;
    //TODO
    if (firstMouse)
    {
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

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 80.0f)
        fov = 80.0f;
    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(fov), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
        startAnimation = false;
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
        startAnimation = false;
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
        startAnimation = false;
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
        startAnimation = false;
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }


}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    scene.LoadModel("models/scena15.obj");
    boat.LoadModel("models/barcaaa.obj");
}


void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
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
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    //lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");

	// send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
	//glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    pos = glm::vec3(44.229f, 16.925f, 2.323f);
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(pos));

}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/posx.jpg");
    faces.push_back("skybox/negx.jpg");
    faces.push_back("skybox/posy.jpg");
    faces.push_back("skybox/negy.jpg");
    faces.push_back("skybox/posz.jpg");
    faces.push_back("skybox/negz.jpg");
    mySkyBox.Load(faces);
    
}

void cameraAnimation() {
    if (startAnimation) {
        const float rotationSpeed = 0.5f;
        const int duration = 50;

        if (startAnimationCount < duration) {
            if (yaw > -145.0f) {
                yaw -= 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 2 * duration) {
            if (pitch > -10.0f) {
                pitch -= 0.5;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 3 * duration) {
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        }
        else if (startAnimationCount < 4 * duration) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        else if (startAnimationCount < 5 * duration) {
            if (yaw < 30.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 6 * duration) {
            if (yaw < 45.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 7 * duration) {
            if (yaw < 60.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 8 * duration) {
            if (yaw < 90.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 9 * duration) {
            if (yaw < 120.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 10 * duration) {
            if (yaw < 150.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (startAnimationCount < 11 * duration) {
            if (yaw < 180.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        
        else {
            startAnimation = false;
        }

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        startAnimationCount++;
    }
    if (startAnimationCount > 600)
         startAnimation= false;
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    //teapot.Draw(shader);
    scene.Draw(shader);
}

//float delta = 0;
float movementSpeed = 2; // units per second
void updateDelta(double elapsedSeconds) {
    //if (delta == 1) delta = 0;
    //else delta = delta + movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

GLfloat index=-0.05;
int counter = 0;
int upC = -1;
void renderBoat(gps::Shader shader) {
    shader.useShaderProgram();
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    //boat.Draw(shader);
    if (boatAnimationForward) {
        if (counter == 100) {
            index = index * (-1);
            upC *= (-1);
        }

        else if (counter == 0) {
            index = index * (-1);

            upC *= (-1);
        }
            delta += index;
            counter += upC;
            //std::cout << index << std::endl;
        
     
    modelMatrixB = glm::mat4(1.0f);
    modelMatrixB = glm::translate(modelMatrixB, glm::vec3(delta, 0, -delta));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrixB));

    glm::mat3 normalMatrixB = glm::mat3(glm::inverseTranspose(view * modelMatrixB));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixB));
    boat.Draw(shader);
    }
    else
    {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        boat.Draw(shader);
    }

    //modelMatrixB = glm::translate(modelMatrixB, glm::vec3(0, 0, delta));
    //glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrixB));
    //boat.Draw(shader);

    //delta += 0.001;
    // get current time
    //if (boatAnimationForward == true)
    //{
    //double currentTimeStamp = glfwGetTime();
    //    updateDelta(currentTimeStamp - lastTimeStamp);
    //    lastTimeStamp = currentTimeStamp;

     //   modelMatrixB = glm::mat4(1.0f);
     //   modelMatrixB = glm::translate(modelMatrixB, glm::vec3(delta, 0, 0));
     //   glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrixB));
     //   glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrixB));
        
      //  for (int i = 0; i < 500; i++) {
            //boat.Draw(shader);
        }
    //}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//render the scene

	// render the teapot
	renderTeapot(myBasicShader);
    renderBoat(myBasicShader);
    mySkyBox.Draw(skyboxShader, view, projection);


    if (startAnimation) cameraAnimation();

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
    initSkyBox();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
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
