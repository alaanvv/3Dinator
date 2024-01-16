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

struct SpotLight {
  vec3 amb, dif, spc, pos, dir;
  float k, l, q, cutoff, outer_cutoff;
};

uniform Material  MAT;
uniform SpotLight LIG;
uniform vec3 P_CAM;

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

void main() {
  vec3 light_dir =   normalize(LIG.pos - pos);
  vec3 view_dir =    normalize(P_CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));

  float theta = dot(light_dir, normalize(-LIG.dir));
  float epsilon = LIG.cutoff - LIG.outer_cutoff;
  float intensity = clamp((theta - LIG.outer_cutoff) / epsilon, 0, 1);

  float distance = length(LIG.pos - pos);
  float attenuation = 1 / (LIG.k + LIG.l * distance + LIG.q * distance * distance);

  vec3 ambient  = attenuation * LIG.amb * vec3(texture(MAT.s_dif, tex)) * MAT.amb;
  vec3 diffuse  = intensity * attenuation * LIG.dif * vec3(texture(MAT.s_dif, tex)) * MAT.dif * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = intensity * attenuation * LIG.spc * vec3(texture(MAT.s_spc, tex)) * MAT.spc * pow(max(dot(view_dir, reflect_dir), 0), MAT.shi);
  color = vec4(MAT.col * (ambient + diffuse + specular), 1);
}
