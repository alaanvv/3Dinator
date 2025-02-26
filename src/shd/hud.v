# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;

uniform mat4 MODEL;
uniform mat4 PROJ;
out vec2 tex;

void main() {
  gl_Position = PROJ * MODEL * vec4(aPos, 1);
  tex = aTex;
}
