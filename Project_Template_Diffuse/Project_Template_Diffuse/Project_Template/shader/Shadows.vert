#version 460

//in variables, this are in model coordinates
layout (location = 0) in vec3 VertexPosition; 
layout (location = 1) in vec3 VertexNormal; 

//out vector needed for the fragment shader
out vec3 Normal;
out vec3 Position;
out vec4 ShadowCoord;

 


//uniforms for matrices required in the shader
uniform mat4 ModelViewMatrix;   //model view matrix
uniform mat3 NormalMatrix;		//normal matrix
uniform mat4 MVP;			
uniform mat4 ShadowMatrix;
//model view projection matrix
 
void main() 
{ 
  //transfrom normal from model coordinates to view coordinates
  Position = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;
  Normal = normalize(NormalMatrix * VertexNormal);
  ShadowCoord = ShadowMatrix * vec4(VertexPosition,1.0);
  gl_Position = MVP * vec4(VertexPosition,1.0); 
} 