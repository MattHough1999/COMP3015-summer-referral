#include "scenebasic_uniform.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"

#include <string>
using glm::vec3;
using glm::mat4;

GLFWwindow* window;
//constructor for torus
SceneBasic_Uniform::SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow) : shadowMapWidth(512),shadowMapHeight(512), plane(20,20,1,1) ,teapot(5, glm::mat4(1.0f)),tPrev(0.0f),lightPos(5.0f,5.0f,5.0f,1.0f)
{
    mesh = ObjMesh::load("../Project_Template/media/spot.obj");
    object = ObjMesh::load("../Project_Template/media/spot.obj");
    object1 = ObjMesh::load("../Project_Template/media/Goblet.obj");
    object2 = ObjMesh::load("../Project_Template/media/trophy.obj");
    object3 = ObjMesh::load("../Project_Template/media/pig_triangulated.obj");
    object4 = ObjMesh::load("../Project_Template/media/spot.obj");

    window = sceneRunnerWindow;
}


//variables used for ImGUI interactions;
float Roughnes = 0.1f, speed, Color[3] = {0.5f,0.5f,0.5f}, LightPos[3], objPos[3], objRot[3], objScale[4] = { 1.0f,1.0f,1.0f ,1.0f }, LightIntensity[3] = { 45.0f,45.0f,45.0f };
bool wireframe,metalic = false;
int objIndex = 0;
const char* objects[5] = {"Spot","Goblet","Trophy","Pig","Custom"};


void SceneBasic_Uniform::initScene()
{
    
   compile();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
   
    angle = glm::quarter_pi<float>();
    setupFBO();
    GLuint programHandle = prog.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

    shadowBias = mat4(glm::vec4(0.5f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.5f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.5f, 0.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    float c = 1.65f;
    vec3 lightPos = vec3(0.0f, c * 5.25, c * 7.5);
    lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
    lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();

    prog.setUniform("Light.Intensity", vec3(0.85));
    prog.setUniform("ShadowMap", 0);

    /*
    view = glm::lookAt(
        glm::vec3(0.0f,4.0f,7.0f),
        glm::vec3(0.0f,0.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f));
    
    projection = glm::perspective(glm::radians(50.f), (float)width / height, 0.5f, 100.0f);
    lightAngle = 0.0f;
    lightRotationSpeed = 1.5f;
    

    prog.setUniform("Light[0].L", glm::vec3(45.0f));
    prog.setUniform("Light[0].Position", view * lightPos);
    prog.setUniform("Light[1].L", glm::vec3(0.3f));
    prog.setUniform("Light[1].Position", glm::vec4(0,0.15f,-1.0f,0));
    prog.setUniform("Light[2].L", glm::vec3(45.0f));
    prog.setUniform("Light[2].Position", view * glm::vec4(-7,3,7,1));
    */
    //IMGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;


    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();

        solidProg.compileShader("shader/Shadows.vert", GLSLShader::VERTEX);
        solidProg.compileShader("shader/Shadows.frag", GLSLShader::FRAGMENT);
        solidProg.link();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	//update your angle here
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) { deltaT = 0.0f; }

    tPrev = t;
    angle += 0.2f * deltaT;
    if(angle > glm::two_pi<float>())
    {
        angle -= glm::two_pi<float>();
    }
    /*
    if(animating())
    {
        lightAngle = glm::mod(lightAngle + deltaT * lightRotationSpeed, glm::two_pi<float>());
        lightPos.x = (glm::cos(lightAngle) * 7.0f) + LightPos[0];
        lightPos.y = 3.0f + LightPos[1];
        lightPos.z = (glm::sin(lightAngle) * 7.0f) + LightPos[2];
    }*/
}

void SceneBasic_Uniform::render()
{
    prog.use();
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.f);

    //prog.setUniform("Light[0].Position", view * lightPos);
    //prog.setUniform("Light[0].L", glm::vec3(LightIntensity[0], LightIntensity[1], LightIntensity[2]));
    drawScene();
    glCullFace(GL_BACK);
    glFlush();
    float c = 2.0f;
    vec3 cameraPos(c * 11.5 * cos(angle), c * 7.0f, c * 11.5 * sin(angle));
    view = glm::lookAt(cameraPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    prog.setUniform("Light.Position", view * glm::vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
    drawScene();

    solidProg.use();
    solidProg.setUniform("Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    solidProg.setUniform("MVP", projection * mv);
    lightFrustum.render();
       
    renderUserInterface();
    //teapot.render();  
}
bool showErr = false;
void SceneBasic_Uniform::renderUserInterface()
{
    //bool showErr = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Shader options
    ImGui::Begin("Shader Menu"); 

    ImGui::Text("This is some text");
    
    ImGui::SliderFloat("Roughness", &Roughnes, 0.0f, 1.0f);
    ImGui::ColorEdit3("Color", Color);
    ImGui::Checkbox("Metalic", &metalic);

    ImGui::End();

    //Light options
    ImGui::Begin("Lights Menu");

    ImGui::Text("This is some more text");
    ImGui::SliderFloat("Rotation Speed", &lightRotationSpeed, 0.0f, 5.0f);
    ImGui::SliderFloat3("Intensity (Colour)", LightIntensity, 0, 100);
    ImGui::SliderFloat3("Position", LightPos, -10.00f, 10.00f);
        

    ImGui::End();

    //Object Menu
    ImGui::Begin("Object Menu");
    ImGui::Text("To load a custom model enter its filename!");
    ImGui::InputText("Your filename",tryFileName,64, ImGuiInputTextFlags_CharsNoBlank);
    if(ImGui::Button("Get File"))
    {
        if(tryFileText())
        {
            objIndex = 4;
            showErr = false;
        }
        else
        {
            showErr = true;
        }
    }
    if(showErr)
    {
        ImGui::Text("Error Loading File, Try again");
    }
    ImGui::Combo("Object loaded", &objIndex, objects, IM_ARRAYSIZE(objects));
    //ImGui::Text("This is even more text");
    
    
    ImGui::SliderFloat("X Pos", &objPos[0],-4.0f,4.0f);
    ImGui::SliderFloat("Y Pos", &objPos[1], -4.0f, 4.0f);
    ImGui::SliderFloat("Z Pos", &objPos[2], -4.0f, 4.0f); ImGui::SameLine();
    if (ImGui::Button("Reset Pos")) {
        objPos[0] = 0;
        objPos[1] = 0;
        objPos[2] = 0;
    }
   
    ImGui::NewLine();
    
    ImGui::SliderFloat("X Scale", &objScale[0], 0.00f, 3.00f);
    ImGui::SliderFloat("Y Scale", &objScale[1], 0.00f, 3.00f);
    ImGui::SliderFloat("Z Scale", &objScale[2], 0.00f, 3.00f);
    ImGui::SliderFloat("W Scale", &objScale[3], 0.00f, 3.00f);
    ImGui::SameLine();
    if (ImGui::Button("Reset Scale")) {
        objScale[0] = 1;
        objScale[1] = 1;
        objScale[2] = 1;
        objScale[3] = 1;
    }

    ImGui::NewLine();
    
    
    ImGui::SliderFloat("X Rot", &objRot[0], -180.0f, 180.0f);
    ImGui::SliderFloat("Y Rot", &objRot[1], -180.0f, 180.0f);
    ImGui::SliderFloat("Z Rot", &objRot[2], -180.0f, 180.0f);
    ImGui::SameLine();
    if (ImGui::Button("Reset Rot")) {
        objRot[0] = 0;
        objRot[1] = 0;
        objRot[2] = 0;

    }


    //ImGui::Checkbox("Wireframe", &wireframe);
    
    

    ImGui::End();


    ImGui::Render();
    int display_w, display_h;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void SceneBasic_Uniform::setMatrices()
{
    glm::mat4 mv = view * model; //we create a model view matrix
    
    prog.setUniform("ModelViewMatrix", mv); //set the uniform for the model view matrix
    
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    //prog.setUniform("NormalMatrix", glm::mat3(mv)); //we set the uniform for normal matrix
    
    prog.setUniform("MVP", projection * mv); //we set the model view matrix by multiplying the mv with the projection matrix

    prog.setUniform("ShadowMatrix", lightPV * model);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    //projection = glm::perspective(glm::radians(60.0f), (float) width / height, 0.3f, 100.0f);
}

void SceneBasic_Uniform::drawScene()
{
    drawFloor();
    int numCows = 9;
    glm::vec3 cowBaseColor(0.1f, 0.33f, 0.97f);
   for(int i = 0; i < numCows; i++)
    {
        float cowX = i * (10.0f / (numCows - 1)) - 5.0f;
        float rough = (i + 1) * (1.0f / numCows);
        drawSpot(glm::vec3(cowX, 0, -6), rough, 0, cowBaseColor);
    }

    float metalRough = 0.43f;

    drawSpot(glm::vec3(-3.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(1, 0.71f, 0.29f));
    drawSpot(glm::vec3(-1.5f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.95f, 0.64f, 0.64f));
    drawSpot(glm::vec3(-0.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.91f, 0.92f, 0.92f));
    drawSpot(glm::vec3(1.5f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.542f, 0.497f, 0.449f));
    drawSpot(glm::vec3(3.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.95f, 0.93f, 0.88f));

    drawCustom(glm::vec3(objPos[0], objPos[1], objPos[2]), Roughnes, metalic, objIndex, glm::vec3(Color[0], Color[1],Color[2]), glm::vec3(objRot[0], objRot[1], objRot[2]) );
}
void SceneBasic_Uniform::drawFloor() 
{
    prog.setUniform("Material.Kd",0.25f,0.25f,0.25f);
    prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
    prog.setUniform("Material.Shininess", 1.0f);

    model = glm::mat4(1.0f);
    setMatrices();
    plane.render();
    /*prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Color", glm::vec3(0.2f));*/
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
    setMatrices();
    plane.render();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 5.0f, -5.0f));
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    plane.render();
    model = mat4(1.0f);
}
void SceneBasic_Uniform::drawSpot(const glm::vec3&pos, float rough, int metal, const glm::vec3&color)
{
    model = glm::mat4(1.0f);
    /*
    prog.setUniform("Material.Rough", rough);
    prog.setUniform("Material.Metal", metal);
    prog.setUniform("Material.Color", color);*/
    prog.setUniform("Material.Kd", 0.25f, 0.25f, 0.25f);
    prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
    prog.setUniform("Material.Shininess", 1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    mesh->render();
}
void SceneBasic_Uniform::drawCustom(const glm::vec3& pos, float rough, int metal, int index, const glm::vec3& color, const glm::vec3& rotation) 
{
    model = glm::mat4(1.0f);
    /*
    prog.setUniform("Material.Rough", rough);
    prog.setUniform("Material.Metal", metal);
    prog.setUniform("Material.Color", color);
    */
    prog.setUniform("Material.Kd", 0.25f, 0.25f, 0.25f);
    prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
    prog.setUniform("Material.Shininess", 1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(objScale[0] * objScale[3], objScale[1] * objScale[3], objScale[2] * objScale[3]));
    model = glm::rotate(model, glm::radians(rotation[0]), vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation[1]), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation[2]), vec3(0.0f, 0.0f, 1.0f));
    

    setMatrices();
    switch (index)
    {
    case 0:
        object->render();
        break;
    case 1:
        object1->render();
        break;
    case 2:
        object2->render();
        break;
    case 3:
        object3->render();
        break;
    case 4:
        object4->render();
        break;
    default:
        break;
    }
}
void SceneBasic_Uniform::setupFBO() 
{
    GLfloat border[] = { 1.0f,0.0f,0.0f,0.0f };
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    GLenum drawbuffers[] = { GL_NONE };
    glDrawBuffers(1, drawbuffers);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(result == GL_FRAMEBUFFER_COMPLETE)
    {
        printf("FrameBuffer Is Complete\n");
    }
    else 
    {
        printf("FrameBuffer Is Not Compete\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
bool SceneBasic_Uniform::tryFileText()
{
    std::string temp(tryFileName);
    std::string fileName = "../Project_Template/media/" + temp;
    FILE* file;
    errno_t err;
    err = fopen_s(&file, fileName.c_str(), "r");
    if (err == 0) 
    {
        object4 = ObjMesh::load(fileName.c_str(), true); return true;
    }
    else 
    {
        fileName = fileName + ".obj";
        err = fopen_s(&file, fileName.c_str(), "r");
        if(err == 0){ object4 = ObjMesh::load(fileName.c_str(), true); return true; }
        object4 = ObjMesh::load("../Project_Template/media/spot.obj");
        return false;
    }
    
    
    return true;
    
}
