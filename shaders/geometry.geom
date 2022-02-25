#version 410 core
layout (points) in;
layout (points, max_vertices = 3) out;
in vec2 TexCoords[];
//out vec2 fTexCoords;

in vec3 color[];

out vec3 fColor;


void main() {    
    fColor = color[0]; 
    vec4 position =gl_in[0].gl_Position;
    //fTexCoords= TexCoords[0];
    gl_Position = position;   
    EmitVertex();   
    EndPrimitive();
    /*
    for(int i = 0;i < gl_in.length();i++)
    {
        fTexCoords=TexCoords[i];
        fColor=color[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        EndPrimitive();

    }
    */


}