# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;
out vec2 _tex;
out vec2 _pos;

void main() {
  _tex = aTex;
  gl_Position = PROJ * VIEW * MODEL * vec4(aPos, 1);
  _pos = vec2(gl_Position);
}
