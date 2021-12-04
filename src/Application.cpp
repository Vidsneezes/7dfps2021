#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArrray.h"

int mainf(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //Sync rate
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "glew not inlcuded" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    {
        //goes in a counter clockwise order 
        //it works as a pen
        float positions[12] = {
            -0.5f, -0.5f, // bottom left
             0.5f, -0.5f,  //bottom right
             0.5f, 0.5f,  // top right
             -0.5f, 0.5f,
        };


        //index buffers
        unsigned int indicies[] = {
            0,1,2,
            2,3,0
        };


        VertexArray va;
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indicies, 6);

        va.UnBind();
        vb.UnBind();
        ib.UnBind();

      //  Shader shader("res/shaders/Basic.shader");
      //  shader.Bind();
      //  shader.SetUniform4f("u_Color", 0.8f, 0.4f, 0.7f, 1.0f);

      //  shader.UnBind();


        float r = 0.0f;
        float increment = 0.05f;
        Renderer renderer;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();

        //    shader.Bind();
          //  shader.SetUniform4f("u_Color", r, 0.4f, 0.7f, 1.0f);
            

            //renderer.Draw(va, ib, shader);

            if (r > 1.0f)
            {
                increment = -0.05f;
            }
            else if (r < 0.0f)
            {
                increment = 0.05f;
            }

            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

    }
    glfwTerminate();
    return 0;
}