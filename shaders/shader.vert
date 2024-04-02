#version 450

layout(push_constant) uniform PushConstants{
    //Model, View, Projection matrix
    mat4 mvp;
} pushConstants;

//Input variable for 3D vertex position
layout(location = 0) in vec3 inPosition;
//Input variable for vertex color
layout(location = 1) in vec3 inColor;
//Input variable for vertex UV
layout(location = 2) in vec2 inTexCoord;

//Output variable that will send color data to the fragment shader using framebuffer with index 0
layout(location = 0) out vec3 fragColor;
//Output variable that will send texture coordinate data to the fragment shader
layout(location = 1) out vec2 fragTexCoord;

void main(){
    //"gl_Position" is a built in variable that acts as the output
    //"gl_VertexIndex" is the index of the current vertex
    //Translate the model in 3D space
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
    //gl_Position = vec4(inPosition, 1.0);

    //Assign the color to the input color
    fragColor = inColor;
    //Assign the texture coordinates
    fragTexCoord = inTexCoord;
}