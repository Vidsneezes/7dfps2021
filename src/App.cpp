#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>

#include "Shader.h"

struct Entity {
    glm::vec3 position;
    float hitTime;
    int life;
};

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void generateCubeVertexData();
void generateBillboardVertexData();
void generateQuadVertexData();
void createTextureData(Shader* _shader);
void RenderEnvironmentCubes(Shader* shader, glm::vec3 walls[], glm::vec3 floors[]);
void RenderBillboards(Shader* shader, glm::vec3 billboards[], glm::vec3 pos);
void UpdateDrawEnemy(Shader* shader);
void Render2dSprite(Shader* shader);
float IntersectCameraRaySphere(const glm::vec3& center, float radius);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 lastCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

float s_width = 640.0f, s_height = 360.0f;

float yaw = -90.0f, pitch = 0.0f;
float fov = 60.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = 400, lastY = 300;
bool firstMouse = true;

//texture data
unsigned int texture1, texture2;
unsigned int billboardVAO, cubeVAO, quadVAO;

Entity enemy;

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(s_width, s_height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    //glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Sync rate
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "glew not inlcuded" << std::endl;
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    glViewport(0, 0, s_width, s_height);

    glEnable(GL_DEPTH_TEST);

    Shader mainShader("res/shaders/vertex.shader", "res/shaders/fragment.shader");
    Shader shader2d("res/shaders/vertex2d.shader", "res/shaders/fragment2d.shader");


    glm::vec3 wallPositions[] = {
    glm::vec3(-1.0f,  0.0f,  1.0f),
    glm::vec3(-0.5f,  0.0f,  1.0f),
    glm::vec3(0.0f,  0.0f,  1.0f),
    glm::vec3(0.5f,  0.0f,  1.0f),
    glm::vec3(1.0f,  0.0f, -1.0f),
    glm::vec3(0.5f,  0.0f, -1.0f),
    glm::vec3(0.0f,  0.0f, -1.0f),
    glm::vec3(-0.5f,  0.0f, -1.0f),

    glm::vec3(1.0f,  0.0f, -0.5f),
    glm::vec3(1.0f,  0.0f, 0.0f),
    glm::vec3(1.0f,  0.0f, 0.5f),
    glm::vec3(-1.0f,  0.0f, -1.0f),
    glm::vec3(-1.0f,  0.0f, -0.5f),
    glm::vec3(-1.0f,  0.0f, 0.0f),
    glm::vec3(-1.0f,  0.0f, 0.5f),

    glm::vec3(1.0f,  0.0f, 1.0f)
    
    };

    glm::vec3 floorPositions[] = {
         glm::vec3(0.5f,  -0.25f,  0.5f),
         glm::vec3(0.0f,  -0.25f,  0.5f),
         glm::vec3(0.5f,  -0.25f,  0.0f),
         glm::vec3(-0.5f,  -0.25f,  -0.5f),
         glm::vec3(-0.5f,  -0.25f,  0.5f),
         glm::vec3(0.5f,  -0.25f,  -0.5f),
         glm::vec3(0.0f,  -0.25f,  -0.5f),
         glm::vec3(-0.5f,  -0.25f,  0.0f),
         glm::vec3(0.0f,  -0.25f,  0.0f),
    };

    enemy.position = glm::vec3(0.0f, 0.0f, 0.0f);
    enemy.hitTime = -1.0f;
    enemy.life = 4;

    createTextureData(&mainShader);
    
    generateCubeVertexData();
    generateBillboardVertexData();
    generateQuadVertexData();


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // 3d scene
        model = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), s_width / s_height, 0.1f, 100.0f);
        glClearColor(0.3f, 0.45f, 0.21f, 1.0f); //state setting
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// state using
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        mainShader.use();
        mainShader.setInt("texture1", 0);

        UpdateDrawEnemy(&mainShader);
        mainShader.setVec2("texCoordOffset", 0.0f, 0.0f);
        RenderEnvironmentCubes(&mainShader, wallPositions, floorPositions);
        
        //2d scene
      
        Render2dSprite(&shader2d);


        glfwSwapBuffers(window);
        glfwPollEvents();

        cameraFront = lastCameraFront;
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 1.7f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;



    cameraPos.y = 0.25f;
//    cameraPos.y = 0.0f;

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        float van = IntersectCameraRaySphere(enemy.position, 0.2f);
        if (van > 0.0f)
        {
            enemy.hitTime = glfwGetTime() + 0.25f;
            enemy.life -= 1;
            std::cout << "mouse clicked" << std::endl;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    lastCameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= yoffset;
    if (fov < 20)
        fov = 20;
    if (fov > 100)
        fov = 100;
}


void generateQuadVertexData() 
{
    //reference quad creationg code https://learnopengl.com/In-Practice/2D-Game/Rendering-Sprites
    unsigned int VBO;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 0.0625f,
        1.0f, 0.0f, 0.0625f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 0.0625f,
        1.0f, 1.0f, 0.0625f, 0.0625f,
        1.0f, 0.0f, 0.0625f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void generateBillboardVertexData() 
{
    float vertices[] = {
        // positions          // texture coords
         0.125f,  0.25f, 0.0f,   0.0625f, 0.6875f, // top right
         0.125f,  0.0f, 0.0f,   0.0625f, 0.6875f - 0.0625f, // bottom right
        -0.125f,  0.0f, 0.0f,   0.0f, 0.6875f - 0.0625f, // bottom left
        -0.125f,  0.25f, 0.0f,   0.0f, 0.6875f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &billboardVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(billboardVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void generateCubeVertexData()
{
    float vertices[] = {
         -0.5f, -0.5f, -0.5f,  0.0f, 1-0.0625f,
          0.5f, -0.5f, -0.5f,  0.0625f, 1-0.0625f,
          0.5f,  0.5f, -0.5f,  0.0625f, 1.0f,
          0.5f,  0.5f, -0.5f,  0.0625f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1-0.0625f,

         -0.5f, -0.5f,  0.5f,  0.0f, 1 - 0.0625f,
          0.5f, -0.5f,  0.5f,  0.0625f, 1 - 0.0625f,
          0.5f,  0.5f,  0.5f,  0.0625f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0625f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 1 - 0.0625f,

         -0.5f,  0.5f,  0.5f,  0.0f, 1 - 0.0625f,
         -0.5f,  0.5f, -0.5f,  0.0625f, 1 - 0.0625f,
         -0.5f, -0.5f, -0.5f,  0.0625f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0625f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1 - 0.0625f,

          0.5f,  0.5f,  0.5f,  0.0f, 1 - 0.0625f,
          0.5f,  0.5f, -0.5f,  0.0625f, 1 - 0.0625f,
          0.5f, -0.5f, -0.5f,  0.0625f, 1.0f,
          0.5f, -0.5f, -0.5f,  0.0625f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1 - 0.0625f,

         -0.5f, -0.5f, -0.5f,  0.0f, 1 - 0.0625f,
          0.5f, -0.5f, -0.5f,  0.0625f, 1 - 0.0625f,
          0.5f, -0.5f,  0.5f,  0.0625f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0625f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1 - 0.0625f,

         -0.5f,  0.5f, -0.5f,  0.0f, 1 - 0.0625f,
          0.5f,  0.5f, -0.5f,  0.0625f, 1 - 0.0625f,
          0.5f,  0.5f,  0.5f,  0.0625f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0625f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1 - 0.0625f,
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    //position data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //color data
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //texture data
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

}

void createTextureData(Shader* _shader)
{
    //load and create texture
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    //texture 1
    unsigned char* data = stbi_load("res/images/MainSpriteSheet.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);


}

void UpdateDrawEnemy(Shader* shader) 
{


    float xTexOff = 0.0f, yTexOff = 0.0f;

    if (enemy.life > 0)
    {
        enemy.position.x = glm::sin(glfwGetTime() * 1.5f) * 0.1f;


        if (glfwGetTime() - enemy.hitTime < 0) {
            xTexOff = 0.0625f;
        }
    }
    else {
        xTexOff = 0.0625f * 2.0f;
    }
    
    shader->setVec2("texCoordOffset", xTexOff, yTexOff);
    RenderBillboards(shader, NULL, enemy.position);

}

void RenderBillboards(Shader* shader, glm::vec3 billboards[], glm::vec3 pos)
{
    model = glm::mat4(1.0f);

    unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
    unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
    unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 look = glm::normalize(cameraPos - pos);
    glm::vec3 right = glm::cross(cameraUp , look);
    model = glm::translate(model, pos);


    model[0] = glm::vec4(right, 0);
    model[1] = glm::vec4(cameraUp, 0);
    model[2] = glm::vec4(look, 0);


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(billboardVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Render2dSprite(Shader* shader) 
{

    shader->use();
    shader->setInt("texture1", 0);
    shader->setVec2("texCoordOffset", 0.0f, 0.0f);


    projection = glm::ortho(0.0f, s_width, 0.0f, s_height, -1.0f, 1.0f);
    model = glm::mat4(1.0f);

    unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
    unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    model = glm::translate(model, glm::vec3(s_width * 0.5f - 24.0f, s_height * 0.5f - 24.0f, 0.0f));
    model = glm::scale(model, glm::vec3(48.0f, 48.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderEnvironmentCubes(Shader *shader, glm::vec3 walls[], glm::vec3 floor[])
{

    unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
    unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
    unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(cubeVAO);


    for (unsigned int j = 0; j < 3; j++)
    {
        for (unsigned int i = 0; i < 16; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, walls[i] + cameraUp * (float)j * 0.5f + cameraUp * 0.25f);
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    for (unsigned int i = 0; i < 9; i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, floor[i]);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

float IntersectCameraRaySphere(const glm::vec3& center, float radius)
{
    glm::vec3 diff = cameraPos - center;
    float a = glm::dot(cameraFront, cameraFront);
    float b = 2.0f * glm::dot(diff, cameraFront);
    float c = glm::dot(diff, diff) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) {
        return -1.0f;
    }
    else {
        return (-b - glm::sqrt(discriminant)) / 2.0f * a;
    }

}

