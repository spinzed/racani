#include "models/Raster.h"

#include <stdlib.h>

Raster::Raster(int width, int height) { set(width, height); }

Raster::~Raster() { del(); }

void Raster::resize(int width, int height) {
    del();
    set(width, height);
}

void Raster::set(int width, int height) {
    this->width = width;
    this->height = height;
    raster = static_cast<float *>(calloc(width * height * 3, sizeof(float)));
}

float *Raster::get() { return raster; }

glm::vec3 Raster::getFragmentColor(int x, int y) {
    // assert((x >= 0 && x < _width) || (y >= 0 && y < _height));
    int h = height - 1 - y;
    int offset = h * width * 3 + x * 3;
    float r = raster[offset];
    float g = raster[offset + 1];
    float b = raster[offset + 2];
    return glm::vec3(r, g, b);
}

void Raster::setFragmentColor(int x, int y, glm::vec3 boja) {
    // assert((x >= 0 && x < _width) || (y >= 0 && y < _height));
    int h = height - 1 - y;
    int offset = h * width * 3 + x * 3;
    raster[offset] = boja.x;
    raster[offset + 1] = boja.y;
    raster[offset + 2] = boja.z;
}

void Raster::del() { free(raster); }