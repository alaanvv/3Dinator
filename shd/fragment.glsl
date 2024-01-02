# version 330 core

uniform float time;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
in vec2 _tex;
in vec2 _pos;
out vec4 color;

void main() {
  color = mix(texture(sampler0, _tex), texture(sampler1, vec2(1 - _tex.x, _tex.y)), (sin(time) + 1) / 2);
}
