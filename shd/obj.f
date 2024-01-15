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


uniform Material MAT;
uniform DirectionalLight LIG;
uniform vec3 P_CAM;

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

void main() {
  vec3 light_dir =  normalize(-LIG.dir);
  vec3 view_dir =   normalize(P_CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));

  vec3 ambient  = LIG.amb * vec3(texture(MAT.s_dif, tex)) * MAT.amb;
  vec3 diffuse  = LIG.dif * vec3(texture(MAT.s_dif, tex)) * MAT.dif * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = LIG.spc * vec3(texture(MAT.s_spc, tex)) * MAT.spc * pow(max(dot(view_dir, reflect_dir), 0), MAT.shi);
  color = vec4(MAT.col * (ambient + diffuse + specular), 1);
}
