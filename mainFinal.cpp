
// Includes ------------------------------------------------------------------------------------------

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

// Fun to Draw the Object´s -------------------------------------------------------------------------------

void drawObject(GLuint vertexBuffer, GLuint uvBuffer, GLuint normalBuffer, size_t vertexCount,
    GLuint MatrixID, glm::mat4 MVP, GLuint ModelMatrixID, glm::mat4 ModelMatrix, GLuint ViewMatrixID, glm::mat4 ViewMatrix) {
    // Send the matrices to shader
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Disable the attributes
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

// Main Fun

int main(void)
{
    // Initialize o GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window 

    window = glfwCreateWindow(1024, 768, "Naves", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Variables  used for the Objective -----------------------------------------------------------------------------------------------------

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // gray background
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Read our .obj file
    std::vector<glm::vec3> hangar_vertices, nave1_vertices, nave2_vertices, hangar2_vertices;
    std::vector<glm::vec2> hangar_uvs, nave1_uvs, nave2_uvs, hangar2_uvs;
    std::vector<glm::vec3> hangar_normals, nave1_normals, nave2_normals, hangar2_normals;

    bool res = loadOBJ("Obj/hangar_modulated.obj", hangar_vertices, hangar_uvs, hangar_normals);
    bool res3 = loadOBJ("Obj/nave_torpedo_modulated.obj", nave1_vertices, nave1_uvs, nave1_normals);
    bool res4 = loadOBJ("Obj/nave_bola_modulated.obj", nave2_vertices, nave2_uvs, nave2_normals);

    hangar2_vertices = hangar_vertices;
    hangar2_uvs = hangar_uvs;
    hangar2_normals = hangar_normals;

    // Buffers for Hangar 1
    GLuint hangar_vertexbuffer, hangar_uvbuffer, hangar_normalbuffer;
    glGenBuffers(1, &hangar_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, hangar_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, hangar_vertices.size() * sizeof(glm::vec3), &hangar_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &hangar_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, hangar_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, hangar_uvs.size() * sizeof(glm::vec2), &hangar_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &hangar_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, hangar_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, hangar_normals.size() * sizeof(glm::vec3), &hangar_normals[0], GL_STATIC_DRAW);

    // Buffers for Nave 1 (nave_torpedo)
    GLuint nave1_vertexbuffer, nave1_uvbuffer, nave1_normalbuffer;
    glGenBuffers(1, &nave1_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave1_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave1_vertices.size() * sizeof(glm::vec3), &nave1_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &nave1_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave1_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave1_uvs.size() * sizeof(glm::vec2), &nave1_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &nave1_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave1_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave1_normals.size() * sizeof(glm::vec3), &nave1_normals[0], GL_STATIC_DRAW);

    // Buffers for Nave 2 (nave_bola)
    GLuint nave2_vertexbuffer, nave2_uvbuffer, nave2_normalbuffer;
    glGenBuffers(1, &nave2_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave2_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave2_vertices.size() * sizeof(glm::vec3), &nave2_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &nave2_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave2_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave2_uvs.size() * sizeof(glm::vec2), &nave2_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &nave2_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nave2_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, nave2_normals.size() * sizeof(glm::vec3), &nave2_normals[0], GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Add rotation for any dynamic effects (optional)
    float rotationSpeed = 0.2f; // Adjustable rotation speed
    float angle = 0.0f;

    // Define global scale and positioning
    float uniformScale = 0.1f;  // Uniform scale for all objects
    float hangarDistance = 300.0f;  // Distance between the hangars
    float naveOffset = 10.0f;       // Offset to position the ships inside the hangars

    // Variables of Movements --------------------------------------------------------------------------------

    bool isMoving = false; // Moving status
    float naveSpeed = 0.5f; // Movement speed
    float nave1PositionX = -hangarDistance; // Initial Position of Nave1 in X 
    float nave2PositionX = hangarDistance;  // Initial Position of Nave2 in X 
    float nave1Rotation = 0.0f;  // Rotation angle for Nave1
    float nave2Rotation = 0.0f;  // Rotation angle for Nave2

    bool isRotatedNave1 = false;  // Rotation Status for Nave1
    bool isRotatedNave2 = false;  // Rotation Status for Nave2

    do {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader
        glUseProgram(programID);

        // Calculate projection and view matrices
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();


        // Reset the app to Initiate Movement
        bool isResetting = false;

        // SPACE key makes the Movement
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isMoving && !isResetting &&
            (nave1PositionX < hangarDistance && nave2PositionX > -hangarDistance)) {
            isMoving = true; // Ativa o movimento
        }

        // Nave x Arrives to the destination -> movement stop
        if (nave1PositionX >= hangarDistance || nave2PositionX <= -hangarDistance) {
            isMoving = false; // Stop movement

            // Rotate Nave
            if (!isRotatedNave1) {
                isRotatedNave1 = true;
            }
            if (!isRotatedNave2) {
                isRotatedNave2 = true;
            }
        }

        // Reset The Nave 1 and Nave 2 with SPACE Key
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isMoving && !isResetting) {
            isResetting = true; 

            // Reset the Variables
            nave1PositionX = -hangarDistance;  
            nave2PositionX = hangarDistance;  
            isRotatedNave1 = false;  
            isRotatedNave2 = false;  
            isMoving = false;  
        }

        // Updates the Position of Nave while Moving
        if (isMoving) {
            nave1PositionX += naveSpeed;
            nave2PositionX -= naveSpeed;
        }

        // Updates the Position of Nave while Moving
        if (isMoving) {
            nave1PositionX += naveSpeed; 
            nave2PositionX -= naveSpeed; 

            // Nave 1 and Nave 2 reach the Opose Hnagar
            if (nave1PositionX >= hangarDistance || nave2PositionX <= -hangarDistance) {
                isMoving = false; 

                // Rotate Naves
                if (!isRotatedNave1) {
                    isRotatedNave1 = true;  
                }
                if (!isRotatedNave2) {
                    isRotatedNave2 = true;  
                }
            }
        }

        // ** Transformation for Hangar 1 **
        glm::mat4 ModelMatrixHangar1 = glm::mat4(1.0f);
        ModelMatrixHangar1 = glm::scale(ModelMatrixHangar1, glm::vec3(uniformScale)); // Uniform scaling
        ModelMatrixHangar1 = glm::translate(ModelMatrixHangar1, glm::vec3(-hangarDistance, 0.0f, 0.0f)); // Move Hangar 1
        glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrixHangar1;

        // Render Hangar 1
        drawObject(hangar_vertexbuffer, hangar_uvbuffer, hangar_normalbuffer, hangar_vertices.size(),
            MatrixID, MVP1, ModelMatrixID, ModelMatrixHangar1, ViewMatrixID, ViewMatrix);

        // ** Transformation for Hangar 2 **
        glm::mat4 ModelMatrixHangar2 = glm::mat4(1.0f);
        ModelMatrixHangar2 = glm::scale(ModelMatrixHangar2, glm::vec3(uniformScale)); // Uniform scaling
        ModelMatrixHangar2 = glm::translate(ModelMatrixHangar2, glm::vec3(hangarDistance, 0.0f, 0.0f)); // Move Hangar 2
        ModelMatrixHangar2 = glm::rotate(ModelMatrixHangar2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 180 degrees
        glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrixHangar2;

        // Render Hangar 2
        drawObject(hangar_vertexbuffer, hangar_uvbuffer, hangar_normalbuffer, hangar_vertices.size(),
            MatrixID, MVP2, ModelMatrixID, ModelMatrixHangar2, ViewMatrixID, ViewMatrix);

        // ** Transformation for Nave 1 (nave_torpedo) **
        glm::mat4 ModelMatrixNave1 = glm::mat4(1.0f);
        ModelMatrixNave1 = glm::scale(ModelMatrixNave1, glm::vec3(uniformScale)); // Uniform scaling
        ModelMatrixNave1 = glm::translate(ModelMatrixNave1, glm::vec3(nave1PositionX, -10.0f, 30.0f)); // Updates Nave1 Position
        ModelMatrixNave1 = glm::rotate(ModelMatrixNave1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotates Nave 1

        // Rotates Nave 1 after it arrives Destination
        if (isRotatedNave1) {
            ModelMatrixNave1 = glm::rotate(ModelMatrixNave1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotates Nave 1
        }

        glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrixNave1;

        // Render Nave 1
        drawObject(nave1_vertexbuffer, nave1_uvbuffer, nave1_normalbuffer, nave1_vertices.size(),
            MatrixID, MVP3, ModelMatrixID, ModelMatrixNave1, ViewMatrixID, ViewMatrix);

        // ** Transformation for Nave 2 (nave_bola) **
        glm::mat4 ModelMatrixNave2 = glm::mat4(1.0f);
        ModelMatrixNave2 = glm::scale(ModelMatrixNave2, glm::vec3(uniformScale)); // Uniform scaling
        ModelMatrixNave2 = glm::translate(ModelMatrixNave2, glm::vec3(nave2PositionX, 15.0f, -30.0f)); // Updates Nave2 Position

        // Rotates Nave 2 after it arrives  Destination
        if (isRotatedNave2) {
            ModelMatrixNave2 = glm::rotate(ModelMatrixNave2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotates Nave 2
        }

        glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrixNave2;

        // Render Nave 2
        drawObject(nave2_vertexbuffer, nave2_uvbuffer, nave2_normalbuffer, nave2_vertices.size(),
            MatrixID, MVP4, ModelMatrixID, ModelMatrixNave2, ViewMatrixID, ViewMatrix);


        //--------------------------------------------------------------------luzes
       
 
        // Set Light inside the 2 Hangars
        glm::vec3 LightPositionHangar1 = glm::vec3(-hangarDistance, 10.0f, 0.0f); // Dentro do hangar 1
        glm::vec3 LightPositionHangar2 = glm::vec3(hangarDistance, 10.0f, 0.0f);  // Dentro do hangar 2


     
        // Position Light -> shader
        GLuint LightIDHangar1 = glGetUniformLocation(programID, "LightPositionHangar1");
        GLuint LightIDHangar2 = glGetUniformLocation(programID, "LightPositionHangar2");

        // send position hangar 1
        glUniform3fv(LightIDHangar1, 1, &LightPositionHangar1[0]);

        // send position hangar 2
        glUniform3fv(LightIDHangar2, 1, &LightPositionHangar2[0]);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);



    // Cleanup buffers for Hangar 1
    glDeleteBuffers(1, &hangar_vertexbuffer);
    glDeleteBuffers(1, &hangar_uvbuffer);
    glDeleteBuffers(1, &hangar_normalbuffer);

    // Cleanup buffers for Nave 1
    glDeleteBuffers(1, &nave1_vertexbuffer);
    glDeleteBuffers(1, &nave1_uvbuffer);
    glDeleteBuffers(1, &nave1_normalbuffer);

    // Cleanup buffers for Nave 2
    glDeleteBuffers(1, &nave2_uvbuffer);
    glDeleteBuffers(1, &nave2_normalbuffer);

    // Cleanup VAO
    glDeleteVertexArrays(1, &VertexArrayID);

    // Cleanup shader program
    glDeleteProgram(programID);

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
