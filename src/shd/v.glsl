# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;
out vec3 pos;
out vec3 nrm;

void main() {
  pos = vec3(MODEL * vec4(aPos, 1));
  nrm = mat3(transpose(inverse(MODEL))) * aNrm;
  gl_Position = PROJ * VIEW * MODEL * vec4(aPos, 1);
}
