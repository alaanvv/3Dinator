# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 MODEL;
uniform mat4 PROJ;
uniform int TILE_AMOUNT;
uniform int TILE;
out vec2 tex;

void main() {
  gl_Position = PROJ * MODEL * vec4(aPos, 1);
  tex = vec2((aTex.x + TILE) / max(TILE_AMOUNT, 1), aTex.y);
}
