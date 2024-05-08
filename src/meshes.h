#include <stdio.h>
#include <stdlib.h>

typedef f32 Vertex[8];

typedef struct {
  u32 size, VAO, VBO;
  Vertex* vertexes;
  mat4 model_matrix;
} Model;

Vertex* model_parse(const c8* path, u32* size) {
  vec3*   poss = malloc(sizeof(vec3));
  vec3*   nrms = malloc(sizeof(vec3));
  vec2*   texs = malloc(sizeof(vec2));
  Vertex* vrts = malloc(sizeof(Vertex));

  u32 pos_i = 0;
  u32 nrm_i = 0;
  u32 tex_i = 0;
  u32 vrt_i = 0;

  FILE* file = fopen(path, "r");
  c8 buffer[256];
  while (fgets(buffer, 256, file)) {
    if      (buffer[0] == 'v' && buffer[1] == ' ') {
      poss = realloc(poss, sizeof(vec3) * (++pos_i + 1));
      sscanf(buffer, "v  %f %f %f", &poss[pos_i][0], &poss[pos_i][1], &poss[pos_i][2]);
    } 
    else if (buffer[0] == 'v' && buffer[1] == 'n') {
      nrms = realloc(nrms, sizeof(vec3) * (++nrm_i + 1));
      sscanf(buffer, "vn %f %f %f", &nrms[nrm_i][0], &nrms[nrm_i][1], &nrms[nrm_i][2]);
    }
    else if (buffer[0] == 'v' && buffer[1] == 't') {
      texs = realloc(texs, sizeof(vec2) * (++tex_i + 1));
      sscanf(buffer, "vt %f %f",    &texs[tex_i][0], &texs[tex_i][1]);
    }
    else if (buffer[0] == 'f') { 
      i32 v1[] = {  0, -1, 0 };
      i32 v2[] = {  0, -1, 0 };
      i32 v3[] = {  0, -1, 0 };
      i32 v4[] = { -1, -1, 0 };

      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d",          &v1[0],         &v1[2], &v2[0],         &v2[2], &v3[0],         &v3[2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",          &v1[0], &v1[1], &v1[2], &v2[0], &v2[1], &v2[2], &v3[0], &v3[1], &v3[2]);
      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d %d/  /%d", &v1[0],         &v1[2], &v2[0],         &v2[2], &v3[0],         &v3[2], &v4[0],         &v4[2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &v1[0], &v1[1], &v1[2], &v2[0], &v2[1], &v2[2], &v3[0], &v3[1], &v3[2], &v4[0], &v4[1], &v4[2]);

      if (v4[0] == -1) { // TRI
        vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 3)); // I foresee a segfault

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v2[0]][0];
        vrts[vrt_i][1] = poss[v2[0]][1];
        vrts[vrt_i][2] = poss[v2[0]][2];
        vrts[vrt_i][3] = nrms[v2[2]][0];
        vrts[vrt_i][4] = nrms[v2[2]][1];
        vrts[vrt_i][5] = nrms[v2[2]][2];
        vrts[vrt_i][6] = texs[v2[1]][0];
        vrts[vrt_i][7] = texs[v2[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;
      }
      else {
        vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 6)); // I foresee a segfault

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v2[0]][0];
        vrts[vrt_i][1] = poss[v2[0]][1];
        vrts[vrt_i][2] = poss[v2[0]][2];
        vrts[vrt_i][3] = nrms[v2[2]][0];
        vrts[vrt_i][4] = nrms[v2[2]][1];
        vrts[vrt_i][5] = nrms[v2[2]][2];
        vrts[vrt_i][6] = texs[v2[1]][0];
        vrts[vrt_i][7] = texs[v2[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v4[0]][0];
        vrts[vrt_i][1] = poss[v4[0]][1];
        vrts[vrt_i][2] = poss[v4[0]][2];
        vrts[vrt_i][3] = nrms[v4[2]][0];
        vrts[vrt_i][4] = nrms[v4[2]][1];
        vrts[vrt_i][5] = nrms[v4[2]][2];
        vrts[vrt_i][6] = texs[v4[1]][0];
        vrts[vrt_i][7] = texs[v4[1]][1];
        vrt_i++;
      }
    }
  }
  fclose(file);
  free(poss);
  free(nrms);
  free(texs);

  *size = vrt_i;
  return vrts;
}

Model* model_create(const c8* path) {
  Model* model = malloc(sizeof(Model));
  model->vertexes = model_parse(path, &model->size);

  model->VAO = canvas_create_VAO();
  model->VBO = canvas_create_VBO(model->size * sizeof(Vertex), model->vertexes, GL_STATIC_DRAW);
  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  return model;
}

void model_draw(Model* model, u32 shader) {
  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  glBindVertexArray(model->VAO);
  canvas_unim4(shader, "MODEL", model->model_matrix[0]);
  glDrawArrays(GL_TRIANGLES, 0, model->size);
}
