#version 330 core

uniform vec3 COL;

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

void main() {
    color = vec4(COL, 1);
}
