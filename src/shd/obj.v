# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
layout (location = 2) in vec2 aTex;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;
uniform vec2 TEX_SCALE;
uniform vec2 TEX_INNSET;
uniform vec2 TEX_OUTSET;
out vec3 pos;
out vec3 nrm;
out vec2 tex;

void main() {
  pos = vec3(MODEL * vec4(aPos, 1));
  nrm = aNrm;
  tex = aTex + TEX_INNSET;
  tex *= TEX_SCALE.s == 0 ? vec2(1) : TEX_SCALE;
  if (TEX_OUTSET.s != 0) tex = tex - (TEX_OUTSET * floor(tex / TEX_OUTSET));
  tex.t = 1 - tex.t;
  gl_Position = PROJ * VIEW * MODEL * vec4(aPos, 1);
}
