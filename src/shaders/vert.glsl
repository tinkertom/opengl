#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 col;

out vec3 v_col;

void main()
{
    v_col = col;
    gl_Position = vec4(pos, 0.0f, 1.0f);
}
