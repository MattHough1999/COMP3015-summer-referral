#version 460

//in variables, this are in model coordinates
layout (location = 0) in vec3 VertexPosition; 
layout (location = 1) in vec3 VertexNormal; 

//out vector needed for the fragment shader
out vec3 Position; 
out vec3 Normal;
out vec3 VNormal;
out vec3 VPosition;

//uniforms for matrices required in the shader
uniform mat4 ModelViewMatrix;   //model view matrix
uniform mat3 NormalMatrix;		//normal matrix
uniform mat4 MVP;				//model view projection matrix
 
void main() 
{ 
	VNormal = normalize(NormalMatrix * VertexNormal);
	Normal = normalize(NormalMatrix * VertexNormal);

	Position = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;
	VPosition = vec3(ModelViewMatrix * vec4(VertexPosition,1.0));

  //turns any vertex position into model view projection in preparations to 
  //graphics pipeline processes before fragment shader (clipping)
	gl_Position = MVP * vec4(VertexPosition,1.0); 
} 