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

Fractal
```glsl
uniform float time;

in  vec2 _pos;
out vec4 color;

vec3 palette(float t) {
  vec3 a = vec3(0.5, 0.5, 0.5);
  vec3 b = vec3(0.5, 0.5, 0.5);
  vec3 c = vec3(1.0, 1.0, 1.0);
  vec3 d = vec3(0.263,0.416,0.557);

  return a + b*cos( 6.28318*(c*t+d) );
}

void main() {
  float dis = length(_pos) - 0.1;
  dis = abs(4 / dis / 5);
  float col = smoothstep(1, dis, 0.5);
  col = fract(dis * sin(time) * 0.1);
  col = 1 / col;
  color = vec4(palette(col), 1);
}
```
