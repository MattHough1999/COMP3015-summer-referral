#version 460

//in variables, this are in model coordinates
layout (location = 0) in vec3 VertexPosition; 
layout (location = 1) in vec3 VertexNormal; 

//out vector needed for the fragment shader
out vec4 Position; 
out vec3 Normal;
//out vec2 TexCoord;

uniform float Time;
uniform float Freq = 2.5;
uniform float Velocity = 2.5;
uniform float AMP = 0.6;
uniform bool animated = false;

//uniforms for matrices required in the shader
uniform mat4 ModelViewMatrix;   //model view matrix
uniform mat3 NormalMatrix;		//normal matrix
uniform mat4 MVP;				//model view projection matrix
 
void main() 
{ 
	if(animated)
	{
		vec4 pos = vec4(VertexPosition,1.0);
		float u = Freq * pos.x - Velocity * Time;
		pos.y = (AMP * sin(u) + VertexPosition.y);
		vec3 n = vec3(0.0);
		n.xy = normalize(vec2(cos(u),1.0));

		Position = ModelViewMatrix * pos;
		Normal = normalize((NormalMatrix * n) * VertexNormal);
		gl_Position = MVP * pos; 
	}
	else
	{
		Normal = normalize(NormalMatrix * VertexNormal);

		Position = (ModelViewMatrix * vec4(VertexPosition,1.0));

		gl_Position = MVP * vec4(VertexPosition,1.0); 
	}
	
	
} 