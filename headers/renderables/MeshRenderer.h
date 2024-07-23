#pragma once

#include "models/Mesh.h"
#include "renderables/GenericRenderer.h"

#define VBO_NUM 4

class MeshRenderer : public GenericRenderer {
  public:
    MeshRenderer(Mesh *mesh) : GenericRenderer() {
        assert(mesh != nullptr);
        this->mesh = mesh;
        data = {
            BufferInput{&(mesh->vrhovi), 3},
            BufferInput{&(mesh->boje), 3},
            BufferInput{&(mesh->normals), 3},
            BufferInput{&(mesh->textureCoords), 2},
        };

        init(&data, &mesh->indeksi, mesh->getPrimitiveType());
        updateBufferData();

        mesh->addChangeListener(std::bind(&MeshRenderer::onMeshChange, this));
    }

    void onMeshChange() {
        updateBufferData(); // will automatically pull new data and indices
    }

    Mesh *getMesh() { return mesh; }

  private:
    Mesh *mesh;
    std::vector<BufferInput> data;
};
