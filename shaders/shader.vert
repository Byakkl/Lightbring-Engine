#version 450

//Hard-coded, clip space, vertex positions for a triangle
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

//Hard-coded color array
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

//Declare output variable that will send color data to the fragment shader using framebuffer with index 0
layout(location = 0) out vec3 fragColor;

void main(){
    //"gl_Position" is a built in variable that acts as the output
    //"gl_VertexIndex" is the index of the current vertex
    //Default z and w values are given to stay in clip space
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    //Assign the color from the array based on the vertex index
    fragColor = colors[gl_VertexIndex];
}