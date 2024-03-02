#version 450

//Input variable for 2D vertex position
layout(location = 0) in vec2 inPosition;
//Input variable for vertex color
layout(location = 1) in vec3 inColor;

//Output variable that will send color data to the fragment shader using framebuffer with index 0
layout(location = 0) out vec3 fragColor;

void main(){
    //"gl_Position" is a built in variable that acts as the output
    //"gl_VertexIndex" is the index of the current vertex
    //Default z and w values are given to stay in clip space
    gl_Position = vec4(inPosition, 0.0, 1.0);

    //Assign the color to the input color
    fragColor = inColor;
}