#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>
#include <cglm/cglm.h>

#define MAX_BONE_INFLUENCE 4

typedef struct {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  vec3 Tangent;
  vec3 Bitangent;
  i32 m_BoneIDs[MAX_BONE_INFLUENCE];
  f32 m_Weights[MAX_BONE_INFLUENCE];
} Vertex;

typedef struct {
  u32 id;
  char type[20];
  char path[100];
} Texture;

typedef struct {
  u32* indices;
  u32 num_indices;
} Face;

typedef struct {
  Vertex* vertices[4096];
  u32* indices[4096];
  Face* faces[4096];
  Texture* textures[4096];
  u32 num_vertices, num_indices, num_faces, num_textures;
  u32 VAO, VBO, EBO;
} Mesh;

typedef struct {
  Texture* textures_loaded[16];
  u8 num_textures;
  Mesh* meshes[16];
  u8 numMeshes;
  char dir[100];
  u8 gammaCorrection;
} Model;

#include "mesh.h"

// ---

u32 texture_from_file(const char *path, const char *dir, u8 gamma) {
  char filename[200];
  strcpy(filename, dir);
  strcat(filename, "/");
  strcat(filename, path);
  u32 textureID;
  glGenTextures(1, &textureID);
  i32 width, height, nrComponents;
  u8 *data = stbi_load(filename, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if      (nrComponents == 1) format = GL_RED;
    else if (nrComponents == 3) format = GL_RGB;
    else if (nrComponents == 4) format = GL_RGBA;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
  }
  else {
    printf("Texture failed to load at path: %s\n", path);
    stbi_image_free(data);
  }
  return textureID;
}

Texture* model_load_material_textures(Model* model, struct aiMaterial* mat, enum aiTextureType type, const char* type_name) {
  Texture* textures = (Texture*) malloc(aiGetMaterialTextureCount(mat, type) * sizeof(Texture));

  for (u8 i; i < aiGetMaterialTextureCount(mat, type); i++) {
    struct aiString str;
    u8 skip;
    aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);

    for (u8 j; j < model->num_textures; j++) {
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

  for (u32 i; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vec3 vector = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
    glm_vec3_copy(vector, vertex.Position);

    if (mesh->mNormals) {
      vector[0] = mesh->mNormals[i].x;
      vector[1] = mesh->mNormals[i].y;
      vector[2] = mesh->mNormals[i].z;
      glm_vec3_copy(vector, vertex.Normal);
    }

    if (mesh->mTextureCoords[0]) {
      vec2 vec;
      vec[0] = mesh->mTextureCoords[0][i].x; 
      vec[1] = mesh->mTextureCoords[0][i].y;
      glm_vec2_copy(vec, vertex.TexCoords);
      vector[0] = mesh->mTangents[i].x;
      vector[1] = mesh->mTangents[i].y;
      vector[2] = mesh->mTangents[i].z;
      glm_vec3_copy(vector, vertex.Tangent);
      vector[0] = mesh->mBitangents[i].x;
      vector[1] = mesh->mBitangents[i].y;
      vector[2] = mesh->mBitangents[i].z;
      glm_vec3_copy(vector, vertex.Bitangent);
    }
    else glm_vec2_copy((vec2) { 0, 0 }, vertex.TexCoords);

    new_mesh->vertices[i] = &vertex;
  }

  new_mesh->num_faces = mesh->mNumFaces;
  new_mesh->faces[new_mesh->num_faces] = (Face*) malloc(new_mesh->num_faces * sizeof(Face));

  for (u16 i; i < mesh->mNumFaces; i++) {
    struct aiFace face = mesh->mFaces[i];
    new_mesh->faces[i]->num_indices = face.mNumIndices;
    new_mesh->faces[i]->indices = (u32*) malloc(face.mNumIndices * sizeof(unsigned int));
    for (u16 j; j < face.mNumIndices; j++) new_mesh->faces[i]->indices[j] = face.mIndices[j];
  }

  struct aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
  new_mesh->num_textures = 0;
  new_mesh->textures[new_mesh->num_textures] = model_load_material_textures(model, mat, aiTextureType_DIFFUSE, "texture_diffuse");
  new_mesh->num_textures += aiGetMaterialTextureCount(mat, aiTextureType_DIFFUSE);

  Texture* specularMaps = model_load_material_textures(model, mat, aiTextureType_SPECULAR, "texture_specular");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) realloc(new_mesh->textures, (new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_SPECULAR)) * sizeof(Texture));
  for (u8 i; i <aiGetMaterialTextureCount(mat, aiTextureType_SPECULAR); i++) new_mesh->textures[new_mesh->num_textures++] = &specularMaps[i];

  Texture* normalMaps = model_load_material_textures(model, mat, aiTextureType_HEIGHT, "texture_normal");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) realloc(new_mesh->textures, (new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_HEIGHT)) * sizeof(Texture));
  for (u8 i; i < aiGetMaterialTextureCount(mat,aiTextureType_HEIGHT); i++) new_mesh->textures[new_mesh->num_textures++] = &normalMaps[i];

  Texture* heightMaps = model_load_material_textures(model, mat, aiTextureType_AMBIENT, "texture_height");
  new_mesh->textures[new_mesh->num_textures] = (Texture*) realloc(new_mesh->textures, (new_mesh->num_textures + aiGetMaterialTextureCount(mat, aiTextureType_AMBIENT)) * sizeof(Texture));
  for (u8 i; i < aiGetMaterialTextureCount(mat,aiTextureType_AMBIENT); i++) new_mesh->textures[new_mesh->num_textures++] = &heightMaps[i];
}

void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene) {
  for (u8 i; i < node->mNumMeshes; i++) {
    struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh new_mesh;
    model_process_mesh(model, &new_mesh, mesh, scene);
    model->meshes[model->numMeshes++] = &new_mesh;
  }

  for (u8 i; i < node->mNumChildren; i++) model_process_node(model, node->mChildren[i], scene);
}

void model_load_model(Model* model, const char* obj_file_name) {
  char obj_file_path[128];
  sprintf(obj_file_path, "%s%s", model->dir, obj_file_name);

  const struct aiScene* scene = aiImportFile(obj_file_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  ASSERT(!(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode), "Assimp Err");

  model_process_node(model, scene->mRootNode, scene);
}

void model_draw(Model* model, u8 shader) {
  for (u32 i; i < model->numMeshes; i++)
    drawMesh(model->meshes[i], shader);
}
