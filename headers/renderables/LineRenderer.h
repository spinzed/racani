#pragma once

#include "renderables/GenericRenderer.h"

#define VBO_NUM 4

class LineRenderer : public GenericRenderer {
  public:
    LineRenderer() : GenericRenderer() {
        data = {
            BufferInput{&(mesh->vrhovi), 3},
            BufferInput{&(mesh->boje), 3},
        };

        init(&data, &mesh->indeksi, mesh->getPrimitiveType());
        updateBufferData();
    }

  private:
    std::vector<BufferInput> data;
};
