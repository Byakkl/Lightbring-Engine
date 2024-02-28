#version 450

//Declare the output variable for framebuffer with index 0
layout(location = 0) out vec4 outColor;

//Declare input variable for vertex color data using framebuffer with index 0
layout(location = 0) in vec3 fragColor;

void main(){
    outColor = vec4(fragColor, 1.0);
}