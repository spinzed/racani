#pragma once

#include "renderables/GenericRenderer.h"

#include "models/Mesh.h"

class MeshRenderer : public GenericRenderer {
  public:
    MeshRenderer(Mesh *mesh) : GenericRenderer() {
        assert(mesh != nullptr);
        this->mesh = mesh;
        data = {
            BufferInput{&(mesh->vrhovi), 3},
            BufferInput{&(mesh->boje), 3},
        };

        init(&data, &mesh->indeksi, mesh->getPrimitiveType());
        updateBufferData();

        mesh->addChangeListener(std::bind(&MeshRenderer::onMeshChange, this));
    }

    void onMeshChange() {
        setPrimitiveType(mesh->getPrimitiveType());
        updateBufferData(); // will automatically pull new data and indices
    }

    Mesh *getMesh() { return mesh; }

  private:
    Mesh *mesh;
    std::vector<BufferInput> data;
};