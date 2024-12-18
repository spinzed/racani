#pragma once

#include "renderables/GenericRenderer.h"

class PointRenderer : public GenericRenderer {
  public:
    PointRenderer() : GenericRenderer() {
        data = {
            BufferInput{&(mesh->vrhovi), 3},
            BufferInput{&(mesh->boje), 3},
            BufferInput{&(mesh->normals), 3},
            BufferInput{&(mesh->textureCoords), 2},
        };

        init(&data, &mesh->indeksi, mesh->getPrimitiveType());
        updateBufferData();
    }

  private:
    std::vector<BufferInput> data;
};
