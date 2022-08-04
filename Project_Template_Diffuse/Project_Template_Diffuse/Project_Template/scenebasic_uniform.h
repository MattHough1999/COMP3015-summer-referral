#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include <glm/glm.hpp>
#include "GLFW/glfw3.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    
    //Torus torus;
    Plane plane;
    std::unique_ptr<ObjMesh> mesh;
    std::unique_ptr<ObjMesh> object;
    std::unique_ptr<ObjMesh> object1;
    std::unique_ptr<ObjMesh> object2;
    std::unique_ptr<ObjMesh> object3;
    std::unique_ptr<ObjMesh> object4;
    Teapot teapot;

    float tPrev, lightAngle, lightRotationSpeed, angle,time;
    glm::vec4 lightPos;
    char tryFileName[64];

    void setMatrices();
    void compile();
    void drawScene();
    void drawFloor();
    void drawSpot(const glm::vec3& pos, float rough, int metal, const glm::vec3 & color);
    void drawCustom(const glm::vec3& pos, float rough, int metal,int index, const glm::vec3& color,const glm::vec3 &rotation);
    bool tryFileText();
public:
    SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow);

    void initScene();
    void update( float t );
    void render();
    void renderUserInterface();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
