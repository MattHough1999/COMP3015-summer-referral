#version 460

//in variable that receives the diffuse calculation from the vertex shader
uniform struct LightInfo
{
    vec4 Position;
    vec3 Intensity;
}Light;

uniform struct MaterialInfo
{
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;
}Material;

uniform sampler2DShadow ShadowMap;

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;
//out variable, this typical for all fragment shaders
layout (location = 0) out vec4 FragColor;

vec3 phongModelDiffAndSpec()
{
    vec3 n = Normal;
    vec3 s = normalize(vec3(Light.Position) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect(-s,n);
    float sDotN = max(dot(s,n),0.0);
    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
    vec3 spec = vec3(0.0);
    if(sDotN > 0.0){spec = Light.Intensity * Material.Ks * pow(max(dot(r,v),0.0),Material.shininess);}

    return diffuse + spec;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine (RenderPassType)
void shadeWithShadow()
{
    vec3 ambient = Light.Intensity * Material.Ka;
    vec3 diffAndSpec = phongModelDiffAndSpec();
    float shadow = 1.0;
    if(ShadowCoord.z >= 0)
    {
        shadow = textureProj(ShadowMap,ShadowCoord);
    }
    FragColor = vec4 (diffAndSpec * shadow + ambient, 1.0);
    FragColor = pow(FragColor,vec4(1.0/2.2));
}
subroutine(RenderPassType)
void recordDepth(){}
void main()
{
    //we pass LightInyensity to outr FragColor, notice the difference between vector types
    // vec3 and vec4 and how we solved the problem
    //FragColor = vec4(LightIntensity, 1.0);
    RenderPass();
}
