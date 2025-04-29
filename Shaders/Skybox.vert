#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    vec4 pos = u_Projection * u_View * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}