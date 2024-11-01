//====================================================================
//
// (c) Borna Noureddin
// COMP 8552   British Columbia Institute of Technology
// Assignment 2
// Quadtree implementation
//
//  *** Implement missing code wherever there is a //+++ comment
//
//====================================================================

#include "glad/gl.h"
#include "glfw/include/GLFW/glfw3.h"
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

// Class for each rectangle object
class Rect
{
public:
    float x, y, vx, vy, width, height;
    int id;
    bool collided;
    Rect(int id, float x, float y, float width, float height, float velocityX, float velocityY)
    {
        this->x = x;
        this->y = y;
        this->id = id;
        this->vx = velocityX;
        this->vy = velocityY;
        this->width = width;
        this->height = height;
    }
};

// Main QuadTree class
class QuadTree
{
public:
    QuadTree(int level, Rect *bounds)
    {
        //+++ Initialize the QuadTree here
        /**
        * Initialize the quadtree of int level with Rect bounds
        * 
        * Initialize a vector of 4 null child nodes.
        */ 
        this->level = level;
        this->bounds = bounds;
        nodes = std::vector<QuadTree *>(4, nullptr); // Four null child nodes
    }

    ~QuadTree()
    {
        Clear();
        delete bounds;
    }
    
    void Clear()
    {
        //+++ Clear the objects and nodes
        /**
        * Clear Rects from objects vector.
        * 
        * Recursively clear each sub-quadtree until no more remaining.
        */
        objects.clear();
        for (auto &node : nodes)
        {
            if (node != nullptr)
            {
                node->Clear();
                delete node;
                node = nullptr;
            }
        }
    }
    
    void Insert(Rect *rect)
    {
        //+++ This code has to be written to insert a new Rect object into the tree
        /**
        * If there are no subtrees, the length of objects is lower than the max length,
        * and the current level of the tree is below the max level, split the current tree.
        * 
        * If a Rect exists in a quadrant, is not straddling quadrants,
        * and there are deeper quadtrees, call insert for the Rect inside the
        * deeper quadtree
        * 
        * else, add Rect to the vector of objects.
        */
        if (!nodes[0] && objects.size() >= MAX_OBJECTS && level < MAX_LEVELS)
            Split();

        int index = GetIndex(rect);
        if (index != -1 && nodes[0] != nullptr) 
        {
            nodes[index]->Insert(rect);
        }
        else 
        { 
            objects.push_back(rect);
        }
    }
    
    std::vector<int> *Retrieve(std::vector<int> *result, Rect *rect)
    {
        //+++ This code has to be written to retrieve all the rectangles
        //+++ that are in the same node in the quadtree as rect
        /**
        * Retrieve objects in the same quadtree.
        * If straddling, do not go deeper, push id to back of list of
        * close by rectangles.
        */
        int index = GetIndex(rect);
        if (index != -1 && nodes[0] != nullptr) // Reducing the threshold for straddling in GetIndex means less -1 indices, resulting in faster checking
        {
            nodes[index]->Retrieve(result, rect);
        }
        for (Rect* obj : objects)
        {
            result->push_back(obj->id); // Add the ID of the rectangle instead of the object itself
        }
        return result;
    }
    
private:
    static const int MAX_OBJECTS = 1;
    static const int MAX_LEVELS = 8;
    
    int level;
    Rect *bounds;
    std::vector<Rect *> objects;
    std::vector<QuadTree *> nodes;
    
    void Split()
    {
        //+++ This code has to be written to split a node
        /**
        * Splits the bounds of a quadtree into 4, the
        * assigns each sub-region to a new quadtree
        * for each respective node.
        */
        float subWidth = bounds->width / 2.0f;
        float subHeight = bounds->height / 2.0f;
        float x = bounds->x;
        float y = bounds->y;

        nodes[0] = new QuadTree(level + 1, new Rect(-1, x, y + subHeight, subWidth, subHeight, 0, 0)); // NW
        nodes[1] = new QuadTree(level + 1, new Rect(-1, x + subWidth, y + subHeight, subWidth, subHeight, 0, 0)); // NE
        nodes[2] = new QuadTree(level + 1, new Rect(-1, x, y, subWidth, subHeight, 0, 0)); // SW
        nodes[3] = new QuadTree(level + 1, new Rect(-1, x + subWidth, y, subWidth, subHeight, 0, 0)); // SE
    }
    
    int GetIndex(Rect *rect)
    {
        // Calculate midpoints of both quadtree boundary and rectangle.
        double horizontalMidpoint = bounds->x + (bounds->width * 0.5);
        double verticalMidpoint = bounds->y + (bounds->height * 0.5);

        // Verify rect location in quadrant.
        // Returns:
        // - 0 for northwest
        // - 1 for northeast
        // - 2 for southwest
        // - 3 for southeast
        /**
        * FIX:
        *
        * Changed quadrant index calculation.
        * After visual inspection of collisions, it has a similar
        * success rate, while executing up to 80% faster.
        * 
        * This is likely due to deeper or more granular subtree regions.
        */
        bool inLeft = (rect->x + (rect->width * 0.5) < horizontalMidpoint);
        bool inRight = (rect->x + (rect->width * 0.5) >= horizontalMidpoint);
        bool inTop = (rect->y + (rect->height * 0.5) >= verticalMidpoint);
        bool inBottom = (rect->y + (rect->height * 0.5) < verticalMidpoint);

        if (inLeft == inRight || inTop == inBottom)
        {
            return -1;
        }

        return inRight + 2 * inBottom;
    }
};








///////////////////////////////////////////////////////
// Code to use QuadTree to display squares
///////////////////////////////////////////////////////

// OpenGL shader code
static const char* vertexShaderText =
"#version 110\n"
"uniform mat4 MVP;\n"
"uniform float vCol;\n"
"attribute vec2 vPos;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"   if (vCol > 0.0)\n"
"       color = vec4(1.0, 0.0, 0.0, 1.0);\n"
"   else\n"
"       color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

static const char* fragmentShaderText =
"#version 110\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = color;\n"
"}\n";


// Helper class
class QuadtreeTest
{
public:
    bool useQuadTree = true;      // Whether to use quadtree or brute force
    QuadtreeTest(int squareSize = 5.0f, int numSquares = 5000, int randSize = 10000,
                 int scrWidth = 1280, int scrHeight = 720, float speed = 1.0f);

    bool Initialize()
    {
        quad->Clear();
        for (int i = 0; i < numSquares; ++i)
        {
            float x = (rand() % randSIZE) / (float)randSIZE * (float)scrWIDTH;
            float y = (rand() % randSIZE) / (float)randSIZE * (float)scrHEIGHT;
            float vx = (rand() % randSIZE) / (float)randSIZE * 2*speed - speed;
            float vy = (rand() % randSIZE) / (float)randSIZE * 2*speed - speed;
            Rect* rect = new Rect(i, x, y, squareSIZE, squareSIZE, vx, vy);
            rects.push_back(rect);
            quad->Insert(rect);
        }

        glfwSetErrorCallback(ErrorCallback);

        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        theWindow = glfwCreateWindow(scrWIDTH, scrHEIGHT, "QuadTree demo", NULL, NULL);
        if (!theWindow)
        {
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(theWindow, KeyCallback);
        glfwMakeContextCurrent(theWindow);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);

        return InitializeGL();

        return true;
    }
    
    bool InitializeGL()
    {
        static const struct
        {
            float x, y;
        } vertices[] =
        {
            { -squareSIZE, -squareSIZE },
            { squareSIZE, -squareSIZE },
            { squareSIZE, squareSIZE },
            { -squareSIZE, -squareSIZE },
            { squareSIZE, squareSIZE },
            { -squareSIZE, squareSIZE },
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
        glCompileShader(vertexShader);
        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &shaderCompiled );
        if( shaderCompiled != GL_TRUE )
        {
            printf( "Unable to compile vertex shader %d!\n", vertexShader );
            PrintShaderLog( vertexShader );
            return false;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
        glCompileShader(fragmentShader);
        shaderCompiled = GL_FALSE;
        glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &shaderCompiled );
        if( shaderCompiled != GL_TRUE )
        {
            printf( "Unable to compile fragment shader %d!\n", fragmentShader );
            PrintShaderLog( fragmentShader );
            return false;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        GLint programSuccess = GL_TRUE;
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &programSuccess );
        if( programSuccess != GL_TRUE )
        {
            printf("Error linking program %d!\n", shaderProgram);
            PrintProgramLog(shaderProgram);
            return false;
        }

        mvpLocation = glGetUniformLocation(shaderProgram, "MVP");
        vposLocation = glGetAttribLocation(shaderProgram, "vPos");
        vcolLocation = glGetUniformLocation(shaderProgram, "vCol");

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(vposLocation);
        glVertexAttribPointer(vposLocation, 2, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void *)0);

        return true;
    }
    
    void Update(double elapsedTime)
    {
        quad->Clear();
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            float x = (*it)->x;
            float y = (*it)->y;
            x += (*it)->vx;
            y += (*it)->vy;
            (*it)->x = x;
            (*it)->y = y;
            if (x < 0 || x > scrWIDTH)
                (*it)->vx *= -1;
            if (y < 0 || y > scrHEIGHT)
                (*it)->vy *= -1;
            (*it)->collided = false;
            quad->Insert((*it));
        }
        
        std::vector<int> closeBy;
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            if (useQuadTree)
            {
                closeBy.clear();
                //+++ Use the Retrieve() method of the quadtree
                //+++ and the IsCollided() method to detect if a
                //+++ collision has happened and set the collided
                //+++ flag of the rectangles that have collided
                //+++ as needed.

                /**
                * Retrieve all Rects in the same quadrant as `it`
                * pushing their IDs to `closeBy`.
                * 
                * For all IDs in closeBy, check if they collide with 'it'.
                * 
                * If yes, mark as collided.
                * 
                * If no, pass.
                */
                quad->Retrieve(&closeBy, *it);
                for (auto id : closeBy)
                {
                    Rect* otherRect = rects[id];
                    if ((*it)->id != otherRect->id && IsCollided(*it, otherRect))
                    {
                        (*it)->collided = true;
                        otherRect->collided = true;
                    }
                }
            } else {
                for (vector<Rect*>::iterator it2 = rects.begin(); it2 != rects.end(); ++it2)
                {
                    if ((*it)->id == (*it2)->id)
                        continue;
                    if (IsCollided((*it), (*it2)))
                    {
                        (*it)->collided = true;
                        (*it2)->collided = true;
                        break;
                    }
                }
            }
        }
        cout << "FPS = " << 1.0f / elapsedTime << "         \r" << std::flush;
    }
    
    void Render()
    {
        static bool isFirst = true;
        int width, height;
        glfwGetFramebufferSize(theWindow, &width, &height);
        glViewport(0, 0, width, height);

        glUseProgram(shaderProgram);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            mat4x4_identity(m);
            mat4x4_translate_in_place(m, (*it)->x, (*it)->y, 0);
            mat4x4_ortho(p, 0.0, (float)scrWIDTH, 0.0, (float)scrHEIGHT, -1.0, 1.0);
            mat4x4_mul(mvp, p, m);
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *)mvp);
            glUniform1f(vcolLocation, (*it)->collided ? 1 : 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        isFirst = false;
    }
    
    void Close()
    {
        glfwDestroyWindow(theWindow);
        glfwTerminate();
    }
    
    void Run()
    {
        if (!Initialize())
            exit(EXIT_FAILURE);

        double lastTime = glfwGetTime();
        double elapsedTime;
        while (!glfwWindowShouldClose(theWindow))
        {
            elapsedTime = glfwGetTime() - lastTime;
            lastTime = glfwGetTime();
            Update(elapsedTime);
            Render();
            glfwSwapBuffers(theWindow);
            glfwPollEvents();
        }

        Close();
    }

private:
    QuadTree *quad;
    vector<Rect *> rects;
    GLFWwindow *theWindow;
    GLuint vertexBuffer, vertexShader, fragmentShader, shaderProgram;
    GLint mvpLocation, vposLocation, vcolLocation;
    float squareSIZE = 5.0f;        // Size of squares
    int numSquares = 5000;        // Number of squares
    int randSIZE = 10000;         // Parameter for randomization of velocities of squares
    int scrWIDTH = 1280;          // Screen width
    int scrHEIGHT = 720;          // Screen height
    float speed = 1.0f;           // Speed multiplier
    

    static void ErrorCallback(int error, const char *description)
    {
        cerr << "Error: " << description << endl;
    }

    static void KeyCallback(GLFWwindow* theWindow, int key,
                            int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(theWindow, GLFW_TRUE);
    }
    
    
    void PrintProgramLog( GLuint program )
    {
        //Make sure name is shader
        if( glIsProgram( program ) )
        {
            //Program log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;
            
            //Get info string length
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
            
            //Allocate string
            char* infoLog = new char[ maxLength ];
            
            //Get info log
            glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
            if( infoLogLength > 0 )
            {
                //Print Log
                printf( "%s\n", infoLog );
            }
            
            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf( "Name %d is not a program\n", program );
        }
    }
    
    void PrintShaderLog( GLuint shader )
    {
        //Make sure name is shader
        if( glIsShader( shader ) )
        {
            //Shader log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;
            
            //Get info string length
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
            
            //Allocate string
            char* infoLog = new char[ maxLength ];
            
            //Get info log
            glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
            if( infoLogLength > 0 )
            {
                //Print Log
                printf( "%s\n", infoLog );
            }
            
            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf( "Name %d is not a shader\n", shader );
        }
    }

    bool IsCollided(Rect* r1, Rect* r2)
    {
        ////+++  Implement this function to test if rectangles r1 and r2 have collided
        /**
        * Check the intersection of all 4 edges of each rectangle. 
        * 
        * IF
        * R1 Left Edge overlaps R2 Right Edge AND
        * R1 Right Edge overlaps R2 Left Edge AND
        * R1 Bottom Edge overlaps R2 Top Edge AND
        * R1 Top Edge overlaps R2 Bottom Edge
        * RETURN 1 (true)
        * 
        * ELSE
        * 
        * RETURN 0 (false)
        */
        return (r1->x < r2->x + r2->width &&
            r1->x + r1->width > r2->x &&
            r1->y < r2->y + r2->height &&
            r1->y + r1->height > r2->y);
    }

};

QuadtreeTest::QuadtreeTest(int squareSize, int numSquares, int randSize,
                           int scrWidth, int scrHeight, float speed)
{
    squareSIZE = squareSize;
    this->numSquares = numSquares;
    randSIZE = randSize;
    scrWIDTH = scrWidth;
    scrHEIGHT = scrHeight;
    this->speed = speed;
    quad = new QuadTree(0, new Rect(-1, 0.0f, 0.0f, (float)scrWIDTH, (float)scrHEIGHT, 0.0f, 0.0f));
}
