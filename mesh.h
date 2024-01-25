#include <glad/glad.h>
#include <cglm/cglm.h>
#include <string.h>
#include <stdlib.h>

Mesh* createMesh(Vertex* vertices, unsigned int* indices, Texture* textures, int numVertices, int numIndices, int numTextures) {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    mesh->vertices[mesh->num_vertices] = (Vertex*)malloc(numVertices * sizeof(Vertex));
    mesh->indices[mesh->num_indices] = (unsigned int*)malloc(numIndices * sizeof(unsigned int));
    mesh->textures[mesh->num_textures] = (Texture*)malloc(numTextures * sizeof(Texture));
    memcpy(mesh->vertices, vertices, numVertices * sizeof(Vertex));
    memcpy(mesh->indices, indices, numIndices * sizeof(unsigned int));
    memcpy(mesh->textures, textures, numTextures * sizeof(Texture));
    mesh->VAO = 0;
    return mesh;
}

void setupMesh(Mesh* mesh) {
    glGenVertexArrays(1, &(mesh->VAO));
    glGenBuffers(1, &(mesh->VBO));
    glGenBuffers(1, &(mesh->EBO));
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->vertices), mesh->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->indices), mesh->indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}

void drawMesh(Mesh* mesh, u8 shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < mesh->num_textures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        char number[10];
        char* name = mesh->textures[i]->type;
        if (strcmp(name, "texture_diffuse") == 0)
            sprintf(number, "%d", diffuseNr++);
        else if (strcmp(name, "texture_specular") == 0)
            sprintf(number, "%d", specularNr++);
        else if (strcmp(name, "texture_normal") == 0)
            sprintf(number, "%d", normalNr++);
        else if (strcmp(name, "texture_height") == 0)
            sprintf(number, "%d", heightNr++);
        glUniform1i(glGetUniformLocation(shader, strcat(name, number)), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
    }
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, sizeof(mesh->indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}
