# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
out vec2 _tex;
out vec2 _pos;

void main() {
  _tex = aTex;
  _pos = aPos.xy;

  gl_Position = vec4(aPos, 1);
}
