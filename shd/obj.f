# version 330 core

struct Material {
  vec3 COL, AMB, DIF, SPC;
  float SHI;
  sampler2D S_DIF, S_SPC, S_EMT;
};

struct Light {
  vec3 COL, POS;
};

struct DirectionalLight {
  vec3 COL, DIR;
};

struct PointLight {
  vec3 COL, POS;
  float CON, LIN, QUA;
};

struct SpotLight {
  vec3 COL, POS, DIR;
  float CON, LIN, QUA, INN, OUT;
};

uniform Material  MAT;
uniform SpotLight LIG;
uniform vec3 CAM;

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

void main() {
  vec3 light_dir =   normalize(LIG.POS - pos);
  vec3 view_dir =    normalize(CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));

  float theta = dot(light_dir, normalize(-LIG.DIR));
  float epsilon = LIG.INN - LIG.OUT;
  float intensity = clamp((theta - LIG.OUT) / epsilon, 0, 1);

  float distance = length(LIG.POS - pos);
  float attenuation = 1 / (LIG.CON + LIG.LIN * distance + LIG.QUA * distance * distance);

  vec3 ambient  = attenuation * LIG.COL * vec3(texture(MAT.S_DIF, tex)) * MAT.AMB;
  vec3 diffuse  = intensity * attenuation * LIG.COL * vec3(texture(MAT.S_DIF, tex)) * MAT.DIF * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = intensity * attenuation * LIG.COL * vec3(texture(MAT.S_SPC, tex)) * MAT.SPC * pow(max(dot(view_dir, reflect_dir), 0), MAT.SHI);
  color = vec4(MAT.COL * (ambient + diffuse + specular), 1);
}
