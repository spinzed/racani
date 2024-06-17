#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

class Raster {
    public:
        int width, height;
        float *raster;
        Raster(int width, int height);
        ~Raster();
        void resize(int width, int height);
        glm::vec3 getFragmentColor(int x, int y);
        void setFragmentColor(int x, int y, glm::vec3 boja);
        float* get();

    private:
        void set(int width, int height);
        void del();
};
