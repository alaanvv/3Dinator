# version 330 core

uniform sampler2D TEXTURE_0;
uniform sampler2D TEXTURE_1;
in vec2 _tex;
in vec2 _pos;
out vec4 color;

void main() {
  color = texture(TEXTURE_0, _tex);
}
