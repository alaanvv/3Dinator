# version 330 core

uniform vec3 P_CAM;
uniform vec3 P_LIG;
uniform vec3 C_LIG;
uniform vec3 C_OBJ;
uniform int  SPC_SHINE;
in  vec3 nrm;
in  vec3 pos;
out vec4 color;

void main() {
  float amb_strenght = 0.3;
  float dif_strenght = 0.5;
  float spc_strenght = 0.2;

  vec3 light_dir = normalize(P_LIG - pos);
  vec3 view_dir =  normalize(P_CAM - pos);
  vec3 reflect_dir = normalize(reflect(-light_dir, nrm));

  vec3 ambient  = C_LIG * amb_strenght;
  vec3 diffuse  = C_LIG * dif_strenght * max(dot(normalize(nrm), light_dir), 0);
  vec3 specular = C_LIG * spc_strenght * pow(max(dot(view_dir, reflect_dir), 0), SPC_SHINE);
  color = vec4(C_OBJ * (ambient + diffuse + specular), 1);
}
