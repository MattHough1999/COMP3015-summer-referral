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

SceneBasic_Uniform::SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow) : time(0), plane(20,20,200,1) ,tPrev(0.0f),lightPos(5.0f,5.0f,5.0f,1.0f)
{
    //Mesh objects declaration
    mesh = ObjMesh::load("media/spot.obj");
    object = ObjMesh::load("media/spot.obj");
    object1 = ObjMesh::load("media/Goblet.obj");
    object2 = ObjMesh::load("media/trophy.obj");
    object3 = ObjMesh::load("media/pig_triangulated.obj");
    customOBJ = ObjMesh::load("media/spot.obj");

    //Glfw window (Necessary for ImGUI functions)
    window = sceneRunnerWindow;
}


//variables used for ImGUI interactions;
float Roughnes = 0.1f, speed,freq = 2.5f,amplitude = 0.6f,vel = 2.5f, Color[3] = {0.5f,0.5f,0.5f}, LightPos[3], objPos[3], objRot[3], objScale[4] = { 1.0f,1.0f,1.0f ,1.0f }, LightIntensity[3] = { 45.0f,45.0f,45.0f };
bool wireframe,guiAnimated = false,metalic = false;
int objIndex = 0;
const char* objects[5] = {"Spot","Goblet","Trophy","Pig","Custom"};


void SceneBasic_Uniform::initScene()
{
    //Compile shaders and clear colours
    compile();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
   
    //Setup light speed, angle and surface animation angle
    angle = glm::half_pi<float>();
    lightAngle = 0.0f;
    lightRotationSpeed = 1.5f;

    //Set light positions
    prog.setUniform("Light[0].L", glm::vec3(45.0f));
    prog.setUniform("Light[0].Position", view * lightPos);
    prog.setUniform("Light[1].L", glm::vec3(0.3f));
    prog.setUniform("Light[1].Position", glm::vec4(0,0.15f,-1.0f,0));
    prog.setUniform("Light[2].L", glm::vec3(45.0f));
    prog.setUniform("Light[2].Position", view * glm::vec4(-7,3,7,1));

    //ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void SceneBasic_Uniform::compile()
{
    //Attempts to compile the selected shaders
	try {
		prog.compileShader("shader/PBR.vert");
		prog.compileShader("shader/PBR.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
    //Updates tPrev and time
    time = t;
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) { deltaT = 0.0f; }
    tPrev = t;

    //Rotate and move the spotlight around the scene
    if(animating())
    {
        lightAngle = glm::mod(lightAngle + deltaT * lightRotationSpeed, glm::two_pi<float>());
        lightPos.x = (glm::cos(lightAngle) * 7.0f) + LightPos[0];
        lightPos.y = 3.0f + LightPos[1];
        lightPos.z = (glm::sin(lightAngle) * 7.0f) + LightPos[2];
    }
}

void SceneBasic_Uniform::render()
{
    //Empties colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Sets scene viewing angle
    view = glm::lookAt(
        glm::vec3(10.0f * cos(angle), 4.0f, 10.0f * sin(angle)),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(60.f), (float)width / height, 0.3f, 100.0f);
    
    //Updates light position and intensity
    prog.setUniform("Light[0].Position", view * lightPos);
    prog.setUniform("Light[0].L", glm::vec3(LightIntensity[0], LightIntensity[1], LightIntensity[2]));

    //Updates Vertex animation uniforms
    prog.setUniform("Time", time);
    prog.setUniform("Freq", freq);
    prog.setUniform("Velocity", vel);
    prog.setUniform("AMP", amplitude);

    //Draw scene and UI
    drawScene();
    renderUserInterface();
}
bool showErr = false;
void SceneBasic_Uniform::renderUserInterface()
{
    //Creates new ImGUI frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Shader options
    ImGui::Begin("Shader Menu"); 

    ImGui::SliderFloat("Roughness", &Roughnes, 0.0f, 1.0f);
    ImGui::ColorEdit3("Color", Color);
    ImGui::Checkbox("Metalic", &metalic);

    ImGui::End();

    //Light options
    ImGui::Begin("Lights Menu");

    ImGui::SliderFloat("Rotation Speed", &lightRotationSpeed, 0.0f, 5.0f);
    ImGui::SliderFloat3("Intensity (Colour)", LightIntensity, 0, 100);
    ImGui::SliderFloat3("Position", LightPos, -10.00f, 10.00f);

    ImGui::End();

    //Object Menu
    ImGui::Begin("Object Menu");

    //Takes custom file name checks it using tryFileText() and opens the model if sucessful
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
    
    //Position sliders
    ImGui::SliderFloat("X Pos", &objPos[0],-4.0f,4.0f);
    ImGui::SliderFloat("Y Pos", &objPos[1], -4.0f, 4.0f);
    ImGui::SliderFloat("Z Pos", &objPos[2], -4.0f, 4.0f); ImGui::SameLine();
    if (ImGui::Button("Reset Pos")) {
        objPos[0] = 0;
        objPos[1] = 0;
        objPos[2] = 0;
    }   ImGui::NewLine();
    
    //Scale Sliders (W == "Whole")
    ImGui::SliderFloat("X Scale", &objScale[0], 0.00f, 3.00f);
    ImGui::SliderFloat("Y Scale", &objScale[1], 0.00f, 3.00f);
    ImGui::SliderFloat("Z Scale", &objScale[2], 0.00f, 3.00f);
    ImGui::SliderFloat("W Scale", &objScale[3], 0.00f, 3.00f);ImGui::SameLine();
    if (ImGui::Button("Reset Scale")) {
        objScale[0] = 1;
        objScale[1] = 1;
        objScale[2] = 1;
        objScale[3] = 1;
    }    ImGui::NewLine();
    
    //Rotation Sliders
    ImGui::SliderFloat("X Rot", &objRot[0], -180.0f, 180.0f);
    ImGui::SliderFloat("Y Rot", &objRot[1], -180.0f, 180.0f);
    ImGui::SliderFloat("Z Rot", &objRot[2], -180.0f, 180.0f);    ImGui::SameLine();
    if (ImGui::Button("Reset Rot")) {
        objRot[0] = 0;
        objRot[1] = 0;
        objRot[2] = 0;

    }

    ImGui::End();

    //Vertex animation Menu
    ImGui::Begin("Animation");

    ImGui::Checkbox("Animate",&guiAnimated);
    ImGui::SliderFloat("Velocity", &vel, 0.0f, 10.0f);
    ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 1.0f);
    ImGui::SliderFloat("Frequency", &freq, 0.0f, 10.0f);

    ImGui::End();

    //Set peramertres and render the ImGUI frame.
    ImGui::Render();
    int display_w, display_h;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void SceneBasic_Uniform::setMatrices()
{
    glm::mat4 mv = view * model; //We create a model view matrix
    
    prog.setUniform("ModelViewMatrix", mv); //Set the uniform for the model view matrix
    
    prog.setUniform("NormalMatrix", glm::mat3(mv)); //We set the uniform for normal matrix
    
    prog.setUniform("MVP", projection * mv); //We set the model view matrix by multiplying the mv with the projection matrix
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(60.0f), (float) w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::drawScene()
{
    drawFloor();

    //Renders 9 cows (Spot model) with increasing rougnesses
    int numCows = 9;
    glm::vec3 cowBaseColor(0.1f, 0.33f, 0.97f);
    for(int i = 0; i < numCows; i++)
    {
        float cowX = i * (10.0f / (numCows - 1)) - 5.0f;
        float rough = (i + 1) * (1.0f / numCows);
        drawSpot(glm::vec3(cowX, 0, -6), rough, 0, cowBaseColor);
    }

    float metalRough = 0.43f;

    //Renders the spot model 6 times demonstrating different metalic colours
    drawSpot(glm::vec3(-3.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(1, 0.71f, 0.29f));
    drawSpot(glm::vec3(-1.5f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.95f, 0.64f, 0.64f));
    drawSpot(glm::vec3(-0.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.91f, 0.92f, 0.92f));
    drawSpot(glm::vec3(1.5f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.542f, 0.497f, 0.449f));
    drawSpot(glm::vec3(3.0f, 0.0f, -3.0f), metalRough, 1, glm::vec3(0.95f, 0.93f, 0.88f));

    //Renders the custom model
    drawCustom(glm::vec3(objPos[0], objPos[1], objPos[2]), Roughnes, metalic, objIndex, glm::vec3(Color[0], Color[1],Color[2]), glm::vec3(objRot[0], objRot[1], objRot[2]) );
}
void SceneBasic_Uniform::drawFloor() 
{
   prog.setUniform("animated", true);

    model = glm::mat4(1.0f);
    //Sets the plane shader uniforms, places it within the scene and then renders the plane
    prog.setUniform("Material.Rough", 0.9f);
    prog.setUniform("Material.Metal", 0);
    prog.setUniform("Material.Color", glm::vec3(0.2f));
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    setMatrices();
    plane.render();
}
void SceneBasic_Uniform::drawSpot(const glm::vec3&pos, float rough, int metal, const glm::vec3&color)
{
    //Renders the Spot model based on the shader and positioning parameters passed in
    prog.setUniform("animated", false);
    model = glm::mat4(1.0f);
    prog.setUniform("Material.Rough", rough);
    prog.setUniform("Material.Metal", metal);
    prog.setUniform("Material.Color", color);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    mesh->render();
}
void SceneBasic_Uniform::drawCustom(const glm::vec3& pos, float rough, int metal, int index, const glm::vec3& color, const glm::vec3& rotation) 
{
    //Uses the paramters passed in to set shader uniforms, model postion, rotation and scale
    if (guiAnimated) { prog.setUniform("animated", true); }
    else { prog.setUniform("animated", false); }
    model = glm::mat4(1.0f);
    prog.setUniform("Material.Rough", rough);
    prog.setUniform("Material.Metal", metal);
    prog.setUniform("Material.Color", color);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(objScale[0] * objScale[3], objScale[1] * objScale[3], objScale[2] * objScale[3]));
    model = glm::rotate(model, glm::radians(rotation[0]), vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation[1]), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation[2]), vec3(0.0f, 0.0f, 1.0f));
    
    //Renders one of 4 preset objects or the custom file loaded by a user
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
        customOBJ->render();
        break;
    default:
        break;
    }
}
bool SceneBasic_Uniform::tryFileText()
{
    //Checks if the file in the ImGUI box exists
    std::string temp(tryFileName);
    std::string fileName = "media/" + temp;
    FILE* file;
    errno_t err;
    err = fopen_s(&file, fileName.c_str(), "r");
    //If the file exists the custom mesh is loaded
    if (err == 0) 
    {
        customOBJ = ObjMesh::load(fileName.c_str(), true); return true;
    }
    //If not, ".obj" is added to the file name and the same check is performed
    else 
    {
        fileName = fileName + ".obj";
        err = fopen_s(&file, fileName.c_str(), "r");
        //If the file still cannot be found the "custom" mesh is set to the Spot model
        if(err == 0){ customOBJ = ObjMesh::load(fileName.c_str(), true); return true; }
        customOBJ = ObjMesh::load("media/spot.obj");
        return false;
    }
    
    
    return true;
    
}