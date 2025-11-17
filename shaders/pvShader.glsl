#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out vec3 vColor;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vColor = aColor;

    gl_Position = projection * view * worldPos;
    gl_PointSize = pointSize;
}
