#include <string.h>
#include <stdlib.h>

Mesh* create_mesh(Vertex* vertices, u32* indices, Texture* textures, i16 num_vertices, i16 num_indices, i16 num_textures) {
  Mesh* mesh = (Mesh*) malloc(sizeof(Mesh));
  mesh->vertices[mesh->num_vertices] = (Vertex*) malloc(num_vertices * sizeof(Vertex));
  mesh->indices[mesh->num_indices] = (u32*) malloc(num_indices * sizeof(u32));
  mesh->textures[mesh->num_textures] = (Texture*) malloc(num_textures * sizeof(Texture));

  memcpy(mesh->vertices, vertices, num_vertices * sizeof(Vertex));
  memcpy(mesh->indices, indices, num_indices * sizeof(u32));
  memcpy(mesh->textures, textures, num_textures * sizeof(Texture));

  mesh->VAO = 0; // TODO Remove this
  return mesh;
}

void mesh_setup(Mesh* mesh) {
  glGenVertexArrays(1, &(mesh->VAO));
  glGenBuffers(1, &(mesh->VBO));
  glGenBuffers(1, &(mesh->EBO));

  glBindVertexArray(mesh->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->vertices), mesh->vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->indices), mesh->indices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, nrm));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tex));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tan));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, bit));
  glEnableVertexAttribArray(5);
  glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, bones));
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, weights));

  glBindVertexArray(0);
}

void mesh_draw(Mesh* mesh, u8 shader) {
  u32 dif_num, spc_num, nrm_num, hgt_num;

  for (u32 i; i < mesh->num_textures; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    char number[4];
    char* name = mesh->textures[i]->type;

    // TODO Replace == 0 with a not
    if      (strcmp(name, "tex_dif") == 0) sprintf(number, "%d", ++dif_num);
    else if (strcmp(name, "tex_spc") == 0) sprintf(number, "%d", ++spc_num);
    else if (strcmp(name, "tex_nrm") == 0) sprintf(number, "%d", ++nrm_num);
    else if (strcmp(name, "tex_hgt") == 0) sprintf(number, "%d", ++hgt_num);

    glUniform1i(glGetUniformLocation(shader, strcat(name, number)), i);
    glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
  }

  glBindVertexArray(mesh->VAO);
  glDrawElements(GL_TRIANGLES, sizeof(mesh->indices) / sizeof(u32), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}
