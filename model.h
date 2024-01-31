#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>

#define PRINT(...) { printf(__VA_ARGS__); printf("\n"); }

typedef struct {
  vec3 pos;
  vec3 nrm;
  vec2 tex;
  vec3 tan;
  vec3 bit;
  i32  bones[4];
  f32  weights[4];
} Vertex;

typedef struct {
  u32 id;
  char type[32], path[128];
} Texture;

typedef struct {
  u32* indices;
  u32  num_indices;
} Face;

typedef struct {
  Texture* textures[128];
  Vertex*  vertices[128];
  u32*     indices[128];
  Face*    faces[128];
  u32 num_vertices, num_indices, num_faces, num_textures, VAO, VBO, EBO;
} Mesh;

typedef struct {
  u16 num_textures, num_meshes, gamma_correction;
  Texture* textures_loaded[16];
  Mesh*    meshes[16];
  char dir[128];
} Model;

#include "mesh.h"

// ---

u32 texture_from_file(const char *path, const char *dir, u16 gamma) {
  char filename[200];
  sprintf(filename, "%s/%s", dir, path);
  u32 textureID;
  glGenTextures(1, &textureID);
  i32 width, height, num_components;
  u8 *data = stbi_load(filename, &width, &height, &num_components, 0);
  ASSERT(data, "Failed loading texture at path \"%s\"\n", path);

  GLenum format;
  if      (num_components == 1) format = GL_RED;
  else if (num_components == 3) format = GL_RGB;
  else if (num_components == 4) format = GL_RGBA;
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  stbi_image_free(data);
  return textureID;
}

Texture* model_load_material_textures(Model* model, struct aiMaterial* mat, enum aiTextureType type, const char* type_name) {
  Texture* textures = (Texture*) malloc(aiGetMaterialTextureCount(mat, type) * sizeof(Texture));

  for (u16 i=0; i < aiGetMaterialTextureCount(mat, type); i++) {
    struct aiString str;
    u16 skip;
    aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);

    for (u16 j=0; j < model->num_textures; j++) {
      if (strcmp(model->textures_loaded[j]->path, str.data) == 0) {
        textures[i] = *model->textures_loaded[j];
        skip = 1; 
        break;
      }
    }

    if (!skip) {   
      Texture texture;
      texture.id = texture_from_file(str.data, model->dir, 0);
      strcpy(texture.type, type_name);
      strcpy(texture.path, str.data);
      textures[i] = texture;
      model->textures_loaded[model->num_textures++] = &texture;
    }
  }

  return textures;
}

void model_process_mesh(Model* model, Mesh* new_mesh, struct aiMesh* mesh, const struct aiScene* scene) {
  new_mesh->num_vertices = mesh->mNumVertices;
  new_mesh->vertices[new_mesh->num_vertices] = (Vertex*) malloc(new_mesh->num_vertices * sizeof(Vertex));

  for (u32 i=0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vec3 vector = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
    glm_vec3_copy(vector, vertex.pos);

    if (mesh->mNormals) {
      vector[0] = mesh->mNormals[i].x;
      vector[1] = mesh->mNormals[i].y;
      vector[2] = mesh->mNormals[i].z;
      glm_vec3_copy(vector, vertex.nrm);
    }

    if (mesh->mTextureCoords[0]) {
      vec2 vec;
      vec[0] = mesh->mTextureCoords[0][i].x; 
      vec[1] = mesh->mTextureCoords[0][i].y;
      glm_vec2_copy(vec, vertex.tex);
      vector[0] = mesh->mTangents[i].x;
      vector[1] = mesh->mTangents[i].y;
      vector[2] = mesh->mTangents[i].z;
      glm_vec3_copy(vector, vertex.tan);
      vector[0] = mesh->mBitangents[i].x;
      vector[1] = mesh->mBitangents[i].y;
      vector[2] = mesh->mBitangents[i].z;
      glm_vec3_copy(vector, vertex.bit);
    }
    else glm_vec2_copy((vec2) { 0, 0 }, vertex.tex);

    new_mesh->vertices[i] = &vertex;
  }

  new_mesh->num_faces = mesh->mNumFaces;

  for (u16 i=0; i < mesh->mNumFaces; i++) {
    struct aiFace face = mesh->mFaces[i];
    new_mesh->faces[i] = (Face*) malloc(sizeof(Face));
    new_mesh->faces[i]->num_indices = face.mNumIndices;
    new_mesh->faces[i]->indices = (u32*) malloc(face.mNumIndices * sizeof(u32));
    for (u16 j=0; j < face.mNumIndices; j++) new_mesh->faces[i]->indices[j] = face.mIndices[j];
  }

  struct aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
  new_mesh->num_textures = 0; // TODO Remove this line
  new_mesh->textures[new_mesh->num_textures] = model_load_material_textures(model, mat, aiTextureType_DIFFUSE, "tex_dif");
  new_mesh->num_textures += aiGetMaterialTextureCount(mat, aiTextureType_DIFFUSE);

  // TODO Group
  Texture* spcMaps = model_load_material_textures(model, mat, aiTextureType_SPECULAR, "tex_spc");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) malloc((new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_SPECULAR)) * sizeof(Texture*));
  for (u16 i=0; i <aiGetMaterialTextureCount(mat, aiTextureType_SPECULAR); i++) new_mesh->textures[new_mesh->num_textures++] = &spcMaps[i];

  Texture* nrmMaps = model_load_material_textures(model, mat, aiTextureType_HEIGHT, "tex_nrm");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) malloc((new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_HEIGHT)) * sizeof(Texture*));
  for (u16 i=0; i < aiGetMaterialTextureCount(mat,aiTextureType_HEIGHT); i++) new_mesh->textures[new_mesh->num_textures++] = &nrmMaps[i];

  Texture* hgtMaps = model_load_material_textures(model, mat, aiTextureType_AMBIENT, "tex_hgt");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) malloc((new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_AMBIENT)) * sizeof(Texture*));
  for (u16 i=0; i < aiGetMaterialTextureCount(mat,aiTextureType_AMBIENT); i++) new_mesh->textures[new_mesh->num_textures++] = &hgtMaps[i];
}

void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene) {
  PRINT("A");
  for (u16 i=0; i < node->mNumMeshes; i++) {
    struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh new_mesh;
    model_process_mesh(model, &new_mesh, mesh, scene);
    model->meshes[model->num_meshes++] = &new_mesh;
  }

  PRINT("B Entering loop of %i", node->mNumChildren);
  for (u16 i=0; i < node->mNumChildren; i++) { PRINT("S%i", i); model_process_node(model, node->mChildren[i], scene); PRINT("E%i", i); }
  PRINT("C");
}

void model_setup(Model* model, const char* obj_file_name) {
  char obj_file_path[128];
  sprintf(obj_file_path, "%s%s", model->dir, obj_file_name);
  const struct aiScene* scene = aiImportFile(obj_file_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  ASSERT(!(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode), "Assimp Err"); // TODO Stop doing dumb negation

  PRINT("SS");
  model_process_node(model, scene->mRootNode, scene);
  PRINT("SE");
}

void model_draw(Model* model, u16 shader) {
  for (u32 i=0; i < model->num_meshes; i++)
    mesh_draw(model->meshes[i], shader);
}
