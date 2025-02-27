# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
layout (location = 2) in vec2 aTex;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;
uniform int TILE_AMOUNT;
uniform int TILE;
out vec3 pos;
out vec3 nrm;
out vec2 tex;

void main() {
  pos = vec3(MODEL * vec4(aPos, 1));
  nrm = aNrm;
  tex = vec2((aTex.x + TILE) / max(TILE_AMOUNT, 1), aTex.y);
  gl_Position = PROJ * VIEW * MODEL * vec4(aPos, 1);
}
