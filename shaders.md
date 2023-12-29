Fade
```glsl
layout (location = 0) in vec3 aPos;
out vec4 _color;

void main() {
  float light = aPos.x + 0.5;
  gl_Position = vec4(aPos, 1);
  _color = vec4(0.85 * light, 0.8 * light, 0.6 * light, 1);
}
```

Uniform color
```glsl
layout (location = 0) in vec3 aPos;
out     vec4 _color;
uniform vec4 uniform_color;
void main() {
  gl_Position = vec4(aPos, 1);
  _color = uniform_color;
}
```
