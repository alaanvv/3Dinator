# version 330 core

struct Material {
  vec3 col, amb, dif, spc;
  float shi;
  sampler2D s_dif, s_spc, s_emt;
};

struct Light {
  vec3 amb, dif, spc, pos;
};

struct DirectionalLight {
  vec3 amb, dif, spc, dir;
};

struct PointLight {
  vec3 amb, dif, spc, pos;
  float k, l, q;
};

uniform Material MAT;
uniform PointLight LIG;
uniform vec3 P_CAM;

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

void main() {
  vec3 light_dir =  normalize(LIG.pos - pos);
  vec3 view_dir =   normalize(P_CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));
  float distance = length(LIG.pos - pos);
  float attenuation = 1 / (LIG.k + LIG.l * distance + LIG.q * distance * distance);

  vec3 ambient  = attenuation * LIG.amb * vec3(texture(MAT.s_dif, tex)) * MAT.amb;
  vec3 diffuse  = attenuation * LIG.dif * vec3(texture(MAT.s_dif, tex)) * MAT.dif * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = attenuation * LIG.spc * vec3(texture(MAT.s_spc, tex)) * MAT.spc * pow(max(dot(view_dir, reflect_dir), 0), MAT.shi);
  color = vec4(MAT.col * (ambient + diffuse + specular), 1);
}
