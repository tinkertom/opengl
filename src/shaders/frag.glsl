#version 330 core

uniform vec4 u_col;
out vec4 frag_col;

void main()
{
    frag_col = u_col;
}
