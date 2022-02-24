#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPosition;


// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lightPositionLoc;
GLuint constant;
GLuint linear;
GLuint quadratic;
GLuint shadowMap;

// camera
gps::Camera myCamera(
    glm::vec3(3.0f, 3.0f, 10.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D cube;
gps::Model3D plane;
gps::Model3D sphere;
gps::Model3D monkey;

GLfloat angle;
GLfloat scale;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;

float deltaTime_in_miliSecs;
float currentTimeStamp = 0;
float lastTimeStamp = 0;


bool is_mouseCentered = true;
double last_xpos, last_ypos;
float x_offset, y_offset;
float sensitivity;
float yaw = 0, pitch = 0;

bool wireWiew = false;

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
	//TODO //DONE

    glViewport(0, 0, width, height);
    //glScissor(0, 0, width, height);

    /*WindowDimensions dims;
    dims.width = width;
    dims.height = height;*/

    myWindow.setWindowDimensions({width, height});

    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 40.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    last_xpos = (double)width / 2;
    last_ypos = (double)height / 2;
    is_mouseCentered = true;
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO //DONE

    if (is_mouseCentered) {
        last_xpos = xpos;
        last_ypos = ypos;
        is_mouseCentered = false;
    }

    x_offset = xpos - last_xpos;
    y_offset = last_ypos - ypos;
    last_xpos = xpos;
    last_ypos = ypos;

    sensitivity = 0.1f * deltaTime_in_miliSecs;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    myCamera.rotate(pitch, yaw);

    myBasicShader.useShaderProgram(); //use default shader
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void initUniforms();

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * deltaTime_in_miliSecs);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f * deltaTime_in_miliSecs;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f * deltaTime_in_miliSecs;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_T]) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (pressedKeys[GLFW_KEY_Y]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (pressedKeys[GLFW_KEY_O]) {
        scale += 0.01 * deltaTime_in_miliSecs;

        model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_P]) {
        scale -= 0.01 * deltaTime_in_miliSecs;

        model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_B]) {
        myBasicShader.loadShader(
            "Resource/Shader/solid_vert.shader",
            "Resource/Shader/solid_frag.shader");

        initUniforms();
    }

    if (pressedKeys[GLFW_KEY_N]) {
        myBasicShader.loadShader(
            "Resource/Shader/basic_vert_directional_light.shader",
            "Resource/Shader/basic_frag_directional_light.shader");

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

        initUniforms();
    }

    if (pressedKeys[GLFW_KEY_M]) {
        myBasicShader.loadShader(
            "Resource/Shader/basic_vert_point_light.shader",
            "Resource/Shader/basic_frag_point_light.shader");

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        initUniforms();
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);

    /*hidden mouse cursor and doesn't let it leave the window*/
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    /*hidden mouse cursor and dont let it leave the window*/
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
    teapot.LoadModel("Resource/obj/teapot20segUT.obj");
    cube.LoadModel("Resource/obj/cube.obj");
    sphere.LoadModel("Resource/obj/sphere.obj");
    monkey.LoadModel("Resource/obj/monkey.obj");
    plane.LoadModel("Resource/obj/plane3.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "Resource/Shader/basic_vert_directional_light.shader",
        "Resource/Shader/basic_frag_directional_light.shader");
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
                               0.1f, 40.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(2.0f, 2.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPosition = glm::vec3(-2.0f, 10.0f, -1.0f);
    lightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosition");
    /*send light position to shader*/
    glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPosition));

    constant = glGetUniformLocation(myBasicShader.shaderProgram, "constant");
    glUniform1f(constant, 1.0f);

    linear = glGetUniformLocation(myBasicShader.shaderProgram, "linear_");
    glUniform1f(linear, 0.22f);

    quadratic = glGetUniformLocation(myBasicShader.shaderProgram, "quadratic");
    glUniform1f(quadratic, 0.20f);
}


void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}

void renderCube(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    cube.Draw(shader);
}

void renderSphere(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw sphere
    sphere.Draw(shader);
}

void renderMonkey(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, -2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw monkey
    monkey.Draw(shader);
}

void renderPlane(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    plane.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_STENCIL_BUFFER_BIT);

	// render the teapot
	renderTeapot(myBasicShader);
    renderCube(myBasicShader);
    renderSphere(myBasicShader);
    renderMonkey(myBasicShader);
    renderPlane(myBasicShader);
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
    setWindowCallbacks();

    last_xpos = (double)myWindow.getWindowDimensions().width / 2;
    last_ypos = (double)myWindow.getWindowDimensions().height / 2;
    is_mouseCentered = true;

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        currentTimeStamp = 1000.0f * glfwGetTime() / 20;
        deltaTime_in_miliSecs = currentTimeStamp - lastTimeStamp;

        processMovement();

	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();

        lastTimeStamp = currentTimeStamp;
	}

	cleanup();

    return EXIT_SUCCESS;
}
