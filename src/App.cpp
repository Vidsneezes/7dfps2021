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
    int spriteId;
};

struct Projectile {
    glm::vec3 position;
    glm::vec3 direction;
    int alive;
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
void RenderEntities(Shader* shader, glm::vec3 billboards[], glm::vec3 pos);
void UpdateDrawEnemy(Shader* shader);
void UpdateDrawProjectiles(Shader* shader);
void CollideProjectileEntities();
void Render2dSprite(Shader* shader);
void RenderUISprite(Shader* shader, float xOffset, float yOffset, glm::vec3 position);
float IntersectCameraRaySphere(const glm::vec3& center, float radius);
void RenderRoom(Shader* shader);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 lastCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

float s_width = 640.0f, s_height = 360.0f;

float yaw = -90.0f, pitch = 0.0f;
float fov = 60.0f;

float roomZmin = 0.25f;
float roomZmax = -3.25f;


float roomXmin = -0.25f;
float roomXmax = 3.25f;


float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = 400, lastY = 300;
bool firstMouse = true;

//texture data
unsigned int texture1, texture2;
unsigned int billboardVAO, cubeVAO, quadVAO;

Entity enemy[10];
Projectile projectiles[10];


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

    unsigned int enemyCounter = 0;
    for (enemyCounter = 0; enemyCounter < 10; enemyCounter++) {
        float xPos = 0.5f * (float)(rand() % 7);
        float yPos = -0.5f * (float)(rand() % 7);
        enemy[enemyCounter].position = glm::vec3(xPos, 0.0f, yPos);
        enemy[enemyCounter].hitTime = -1.0f;
        enemy[enemyCounter].life = 4;
        enemy[enemyCounter].spriteId = 4 + rand() % 7;
    }

    for (enemyCounter = 0; enemyCounter < 10; enemyCounter++)
    {
        projectiles[enemyCounter].position = glm::vec3(0.0f, 0.0f, 0.0f);
        projectiles[enemyCounter].alive = 0;
    }
 

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

        CollideProjectileEntities();

        glClearColor(0.05f, 0.07f, 0.11f, 1.0f); //state setting
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// state using
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        mainShader.use();
        mainShader.setInt("texture1", 0);

        UpdateDrawEnemy(&mainShader);
        UpdateDrawProjectiles(&mainShader);

        mainShader.setVec2("texCoordOffset", 0.0f, 0.0f);
        RenderRoom(&mainShader);
        //RenderEnvironmentCubes(&mainShader, wallPositions, floorPositions);
        
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

    float deltaX = 0;
    float deltaZ = 0;

    float cameraSpeed = 1.7f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        deltaZ = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        deltaZ = -cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        deltaX = -cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        deltaX = cameraSpeed;

    float lastX = cameraPos.x;
    float lastZ = cameraPos.z;


    


    cameraPos += cameraFront * deltaZ;
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaX;

    if ((cameraPos).z > roomZmin || (cameraPos).z < roomZmax) {
        cameraPos.z = lastZ;
    }

    if ((cameraPos).x < roomXmin || (cameraPos).x > roomXmax) {
        cameraPos.x = lastX;
    }

    cameraPos.y = 0.25f;
//    cameraPos.y = 0.0f;

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        for (unsigned int i = 0; i < 10; i++)
        {
            if (projectiles[i].alive == 0)
            {
                projectiles[i].position = cameraPos + cameraFront * 0.1f + cameraUp * -0.06f;
                projectiles[i].alive = 1;
                projectiles[i].direction = cameraFront;
                break;
            }
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
         1.0f,  1.0f, 0.0f,   0.0625f, 0.0625f, // top right
         1.0f,  -1.0f, 0.0f,   0.0625f, 0.0f, // bottom right
        -1.0f,  -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   0.0f, 0.0625f  // top left 
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
    
    for (unsigned int i = 0; i < 10; i++)
    {

        float xTexOff = 0.0f, yTexOff = enemy[i].spriteId* 0.0625f;

        if (enemy[i].life > 0)
        {
            //enemy[i].position.x += 0.1f * deltaTime;


            if (glfwGetTime() - enemy[i].hitTime < 0) {
                xTexOff = 0.0625f;
            }
            else {
                enemy[i].hitTime = -1.0f;
            }
        }
        else {
            xTexOff = 0.0625f * 2.0f;
        }

        shader->setVec2("texCoordOffset", xTexOff, yTexOff);
        RenderEntities(shader, NULL, enemy[i].position);
    }
}

void RenderEntities(Shader* shader, glm::vec3 billboards[], glm::vec3 pos)
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

    model = glm::translate(model, glm::vec3(0.0f, 0.16f, 0.0f));
    model = glm::scale(model, glm::vec3(0.16f, 0.16f, 1.0f));


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(billboardVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void UpdateDrawProjectiles(Shader* shader)
{
    for (unsigned int i = 0; i < 10; i++)
    {
        if (projectiles[i].alive == 1)
        {
            float xTexOff = 0.0f, yTexOff = 3* 0.0625f;
            shader->setVec2("texCoordOffset", xTexOff, yTexOff);

            projectiles[i].position += glm::normalize(projectiles[i].direction) * 3.0f * deltaTime;

            if (projectiles[i].position.z > roomZmin 
                || projectiles[i].position.z < roomZmax 
                || projectiles[i].position.x < roomXmin 
                || projectiles[i].position.x > roomXmax
                || projectiles[i].position.y < -0.2f
                || projectiles[i].position.y > 3.0f)
            {
                projectiles[i].alive = 0;
                continue;
            }

            model = glm::mat4(1.0f);

            unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
            unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
            unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            glm::vec3 look = glm::normalize(cameraPos - projectiles[i].position);
            glm::vec3 right = glm::cross(cameraUp, look);
            model = glm::translate(model, projectiles[i].position);

            model[0] = glm::vec4(right, 0);
            model[1] = glm::vec4(cameraUp, 0);
            model[2] = glm::vec4(look, 0);

            model = glm::translate(model, glm::vec3(0.0f, 0.025f, 0.0f));
            model = glm::scale(model, glm::vec3(0.07f, 0.07f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(billboardVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}


void RenderRoom(Shader* shader) {
    float xTexOff = 0.0f, yTexOff = 15.0f * 0.0625f;
    shader->setVec2("texCoordOffset", xTexOff, yTexOff);

    unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
    unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
    unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    unsigned int i = 0, j = 0;

    for (j = 0; j < 7; j++)
    {
        for (i = 0; i < 7; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3((float)i * 0.5f,(float)j *0.5f, 0.0f));

            model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(billboardVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    yTexOff = 14.0f * 0.0625f;
    shader->setVec2("texCoordOffset", xTexOff, yTexOff);
    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < 7; i++)
        {
            if (i != 3 || j > 0) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)i * 0.5f, 0.25f + 0.5f * j, 0.25f));

                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(billboardVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        for (i = 0; i < 7; i++)
        {
            if (i != 3 || j > 0) {
                model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                model = glm::translate(model, glm::vec3((float)i * 0.5f, 0.25f + 0.5f * j, -0.25f));

                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(billboardVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        for (i = 0; i < 7; i++)
        {
            if (i != 3 || j > 0) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)i * 0.5f, 0.25f + 0.5f * j, -0.5f * 7 + 0.25f));

                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(billboardVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        for (i = 0; i < 7; i++)
        {
            if (i != 3 || j > 0) {
                model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                model = glm::translate(model, glm::vec3((float)i * 0.5f, 0.25f + 0.5f * j, 0.5f * 7 - 0.25f));

                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(billboardVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void Render2dSprite(Shader* shader) 
{

    shader->use();
    shader->setInt("texture1", 0);

    projection = glm::ortho(0.0f, s_width, 0.0f, s_height, -1.0f, 1.0f);

    unsigned int projectionLoc = glGetUniformLocation(shader->ID, "projection");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    RenderUISprite(shader, 0.0f, 0.0f, glm::vec3(s_width * 0.5f - 24.0f, s_height * 0.5f - 24.0f, 0.0f));

    for (unsigned int i = 0; i < 3; i++)
    {
        RenderUISprite(shader, 0.0625f, 0.0f, glm::vec3(48.0f * i, 0.0f, 0.0f));
    }

}

void RenderUISprite(Shader* shader, float xOffset, float yOffset, glm::vec3 position)
{
    shader->setVec2("texCoordOffset", xOffset, yOffset);

    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(48.0f, 48.0f, 1.0f));

    unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");

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

void CollideProjectileEntities()
{
    float sphere1Rad = 0.15f * 0.15f;
    float sphere2Rad = 0.2f*0.2f;

    float disX = 0;
    float disY = 0;
    float disZ = 0;

    float sqrDis = 0;


    for (unsigned int i = 0; i < 10; i++)
    {
        if (projectiles[i].alive == 1)
        {
            for (unsigned int j = 0; j < 10; j++)
            {
                if (enemy[j].life > 0) {
                    disX = projectiles[i].position.x - enemy[j].position.x;
                    disY = projectiles[i].position.y - enemy[j].position.y;
                    disZ = projectiles[i].position.z - enemy[j].position.z;

                    disX *= disX;
                    disY *= disY;
                    disZ *= disZ;

                    sqrDis = (disX + disY + disZ);

                    if ((sphere1Rad + sphere2Rad) > sqrDis)
                    {
                        projectiles[i].alive = 0;
                        enemy[j].hitTime = glfwGetTime() + 0.25f;
                        enemy[j].life -= 1;
                    }
                }
            }
        }
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

