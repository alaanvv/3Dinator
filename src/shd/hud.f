# version 330 core

uniform sampler2D S_TEX;

in  vec2 tex;
out vec4 color;

void main() {
  color = texture(S_TEX, tex);
  if (vec3(color) == vec3(0, 1, 0)) { discard; }
  gl_FragDepth = 0;
}
