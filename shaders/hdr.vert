#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
//layout (location = 2) in vec2 aPosP;
//layout (location = 3) in vec3 aColor;

out vec2 TexCoords;
//out vec3 color;
void main()
{


    TexCoords = aTexCoords;
    //color = aColor;
    gl_Position = vec4(aPos, 1.0);
    
}