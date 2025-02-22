# version 330 core

#define DIR_LIG_ENABLE 0
#define PNT_LIG_ENABLE 1
#define SPT_LIG_ENABLE 0

#define DIR_LIG_AMOUNT 1
#define PNT_LIG_AMOUNT 1
#define SPT_LIG_AMOUNT 1

// --- Struct

struct Material {
  vec3 COL;
  sampler2D S_DIF, S_SPC, S_EMT;
  float SHI, AMB, DIF, SPC;
  int LIG;
};

struct DirLig {
  vec3 COL, DIR;
};

struct PntLig {
  vec3  COL, POS;
  float CON, LIN, QUA;
};

struct SptLig {
  vec3  COL, POS, DIR;
  float CON, LIN, QUA, INN, OUT;
};

// --- Setup

uniform vec3 CAM;
uniform vec2 TEX_SCALE;
uniform Material MAT;
uniform DirLig DIR_LIGS[DIR_LIG_AMOUNT];
uniform PntLig PNT_LIGS[PNT_LIG_AMOUNT];
uniform SptLig SPT_LIGS[SPT_LIG_AMOUNT];

in  vec3 nrm;
in  vec3 pos;
in  vec2 tex;
out vec4 color;

// --- Function

vec3 CalcDirLig(DirLig lig, vec3 normal, vec3 cam) {
  vec3 view_dir = normalize(cam - pos);
  vec3 light_dir = normalize(-lig.DIR);

  vec3 ambient = lig.COL * MAT.COL * MAT.AMB;
  ambient *= vec3(texture(MAT.S_DIF, tex));
  ambient += vec3(texture(MAT.S_EMT, tex));

  vec3 diffuse = lig.COL * MAT.COL * MAT.DIF * max(dot(normal, light_dir), 0); 
  diffuse *= vec3(texture(MAT.S_DIF, tex));

  vec3 specular = lig.COL * MAT.COL * MAT.SPC * pow(max(dot(view_dir, reflect(-light_dir, normal)), 0), MAT.SHI);
  specular *= vec3(texture(MAT.S_SPC, tex));

  return (ambient + diffuse + specular);
}

vec3 CalcPntLig(PntLig lig, vec3 normal, vec3 cam, vec3 frag_pos) {
  vec3 view_dir = normalize(cam - pos);
  vec3 light_dir = normalize(lig.POS - frag_pos);

  float distance = length(lig.POS - frag_pos);
  float attenuation = 1 / (lig.CON + lig.LIN * distance + lig.QUA * distance * distance);

  vec3 ambient = attenuation * lig.COL * MAT.COL * MAT.AMB;
  ambient *= vec3(texture(MAT.S_DIF, tex));
  ambient += vec3(texture(MAT.S_EMT, tex));

  vec3 diffuse = attenuation * lig.COL * MAT.COL * MAT.DIF * max(dot(normalize(normal), light_dir), 0); 
  diffuse *= vec3(texture(MAT.S_DIF, tex));

  vec3 specular = attenuation * lig.COL * MAT.COL * MAT.SPC * pow(max(dot(view_dir, reflect(-light_dir, normal)), 0), MAT.SHI);
  specular *= vec3(texture(MAT.S_SPC, tex));

  return (ambient + diffuse + specular);
}

vec3 CalcSptLig(SptLig lig, vec3 normal, vec3 cam, vec3 frag_pos) {
  vec3 view_dir = normalize(cam - pos);
  vec3 light_dir = normalize(lig.POS - frag_pos);

  float theta = dot(light_dir, normalize(-lig.DIR));
  float epsilon = lig.INN - lig.OUT;
  float intensity = clamp((theta - lig.OUT) / epsilon, 0, 1);

  float distance = length(lig.POS - frag_pos);
  float attenuation = 1 / (lig.CON + lig.LIN * distance + lig.QUA * distance * distance);

  vec3 ambient = attenuation * lig.COL * MAT.COL * MAT.AMB;
  ambient *= vec3(texture(MAT.S_DIF, tex));
  ambient += vec3(texture(MAT.S_EMT, tex));

  vec3 diffuse = intensity * attenuation * lig.COL * MAT.COL * MAT.DIF * max(dot(normalize(normal), light_dir), 0); 
  diffuse *= vec3(texture(MAT.S_DIF, tex));

  vec3 specular = intensity * attenuation * lig.COL * MAT.COL * MAT.SPC * pow(max(dot(view_dir, reflect(-light_dir, normal)), 0), MAT.SHI);
  specular *= vec3(texture(MAT.S_SPC, tex));

  return (ambient + diffuse + specular);
}

// --- Main

void main() {
  vec3 _color = vec3(0);

  if (MAT.LIG == 0) {
    if (DIR_LIG_ENABLE == 1)
  for (int i = 0; i < DIR_LIG_AMOUNT; i++)
    _color += CalcDirLig(DIR_LIGS[i], nrm, CAM);

    if (PNT_LIG_ENABLE == 1)
  for (int i = 0; i < PNT_LIG_AMOUNT; i++)
    _color += CalcPntLig(PNT_LIGS[i], nrm, CAM, pos);

    if (SPT_LIG_ENABLE == 1)
  for (int i = 0; i < SPT_LIG_AMOUNT; i++)
    _color += CalcSptLig(SPT_LIGS[i], nrm, CAM, pos);
  }
  else {
    _color = MAT.COL;
  }

    color = vec4(_color, 1);
}
