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

Gouraud
```glsl
# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;
uniform vec3 P_CAM;
uniform vec3 P_LIG;
uniform vec3 C_LIG;
uniform vec3 C_OBJ;

out vec4 _color;

void main() {
  vec3 pos = vec3(MODEL * vec4(aPos, 1));
  vec3 nrm = mat3(transpose(inverse(MODEL))) * aNrm;
  gl_Position = PROJ * VIEW * MODEL * vec4(aPos, 1);

  float amb_strenght = 0.3;
  float dif_strenght = 0.5;
  float spc_strenght = 0.2;

  vec3 light_dir =   normalize(P_LIG - pos);
  vec3 view_dir =    normalize(P_CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));

  vec3 ambient  = C_LIG * amb_strenght;
  vec3 diffuse  = C_LIG * dif_strenght * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = C_LIG * spc_strenght * pow(max(dot(view_dir, reflect_dir), 0), 256);
  _color = vec4(C_OBJ * (ambient + diffuse + specular), 1);
}
```
