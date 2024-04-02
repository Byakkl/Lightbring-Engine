#version 450

//Declare the output variable for framebuffer with index 0
layout(location = 0) out vec4 outColor;

//Declare input variable for vertex color data using framebuffer with index 0
layout(location = 0) in vec3 fragColor;
//Input variable for vertex texture coordinates
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D texSampler;

void main(){
    outColor = vec4(fragColor, 1);//texture(texSampler, fragTexCoord);
}