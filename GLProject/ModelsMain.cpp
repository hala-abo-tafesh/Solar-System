#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "shader.h"
#include "camera.h"
#include "Sphere.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;

const unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
Camera camera(vec3(0.0f, 2.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

bool fastSolar = false;
bool fastLunar = false;
float timeScale = 1.0f;
static float simulatedTime = 0.0f;

// أحجام ومسافات
float sunRadius = 2.0f;
float earthRadius = 0.5f;
float moonRadius = 0.15f;

float distEarthSun = 8.0f;
float distMoon = 0.9;

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << path << "\n";
    }
    stbi_image_free(data);
    return textureID;
}

void framebuffer_size_callback(GLFWwindow* w, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* w, double xposIn, double yposIn) {
    float xpos = (float)xposIn, ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow* w) {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);

    float sideSpeedFactor = 0.5f;

    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * sideSpeedFactor);

    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * sideSpeedFactor);

    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(w, GLFW_KEY_G) == GLFW_PRESS) { fastSolar = true; fastLunar = false; timeScale = 5.0f; }
    if (glfwGetKey(w, GLFW_KEY_H) == GLFW_PRESS) { fastLunar = true; fastSolar = false; timeScale = 5.0f; }
    if (glfwGetKey(w, GLFW_KEY_J) == GLFW_PRESS) { fastSolar = false; fastLunar = false; timeScale = 1.0f; }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (!window) { std::cout << "Failed to create window\n"; return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to init GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");

    Sphere sun(sunRadius);
    Sphere earth(earthRadius);
    Sphere moon(moonRadius);

    unsigned int sunTex = loadTexture("textures/sun.jpg");
    unsigned int earthTex = loadTexture("textures/earth.jpg");
    unsigned int moonTex = loadTexture("textures/moon.jpg");

    shader.use();
    shader.setInt("textureSample", 0);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        simulatedTime += deltaTime * timeScale;
        float t = simulatedTime;

        // --------- Positions ---------
        float earthX = distEarthSun * cos(t * 0.5f);
        float earthZ = distEarthSun * sin(t * 0.5f);
        float moonX = earthX + distMoon * cos(t * 2.2f);
        float moonZ = earthZ + distMoon * sin(t * 2.2f);

        // --------- Eclipse check ---------
        vec2 sunPos2D(0.0f, 0.0f);
        vec2 earthPos2D(earthX, earthZ);
        vec2 moonPos2D(moonX, moonZ);
        vec2 earthToMoon = moonPos2D - earthPos2D;
        vec2 earthToSun = sunPos2D - earthPos2D;
        float cross = abs(earthToMoon.x * earthToSun.y - earthToMoon.y * earthToSun.x);
        bool solarEclipse = (cross < 0.05f && length(moonPos2D) < length(earthPos2D));

        // Intensities
        float sunIntensity = solarEclipse ? 0.05f : 1.2f; // خفف الشمس عند الكسوف
        float moonIntensity = solarEclipse ? 0.25f : 0.15f; // القمر يعطي ضوء خفيف أثناء الكسوف

        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setFloat("sunIntensity", sunIntensity);
        shader.setFloat("moonIntensity", moonIntensity);
        shader.setVec3("sunPos", vec3(0.0f, 0.0f, 0.0f));
        shader.setVec3("sunColor", vec3(1.0f, 0.95f, 0.6f));
        shader.setVec3("moonColor", vec3(0.6f, 0.6f, 0.7f));

        mat4 projection = perspective(radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // --------- Draw SUN ---------
        mat4 modelSun = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
        shader.setMat4("model", modelSun);
        shader.setVec3("objectColor", vec3(1.0f, 0.95f, 0.5f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTex);
        sun.Draw();

        // --------- Draw EARTH ---------
        mat4 earthModel = translate(mat4(1.0f), vec3(earthX, 0.0f, earthZ));
        earthModel = rotate(earthModel, t * 2.0f, vec3(0, 1, 0));
        shader.setMat4("model", earthModel);
        shader.setVec3("objectColor", vec3(0.05f, 0.2f, 0.1f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTex);
        earth.Draw();

        // --------- Draw MOON ---------
        mat4 moonModel = translate(mat4(1.0f), vec3(moonX, 0.0f, moonZ));
        shader.setMat4("model", moonModel);
        shader.setVec3("objectColor", vec3(0.25f, 0.25f, 0.25f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTex);
        moon.Draw();

        if (fastSolar && solarEclipse) {
            fastSolar = false;
            timeScale = 0.0f;
            std::cout << "Solar Eclipse!\n";
        }
        if (fastLunar && cross < 0.05f && length(earthPos2D) < length(moonPos2D)) {
            fastLunar = false;
            timeScale = 0.0f;
            std::cout << "Lunar Eclipse!\n";
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
