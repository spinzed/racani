#pragma once

#include <assimp/scene.h>

#include <string>
#include <vector>
#include <functional>

class ResourceProcessor {
    public:
        virtual void processResource(std::string name, const aiScene* resource) = 0;
};

class Importer {
    public:
    
        static bool loadResource(std::string name, ResourceProcessor *p); // load the entire resource
        static void setPath(std::string path);
        static unsigned int loadTexture(std::string resourceName, std::string fileName);
        static std::string getFilePath(std::string name);

    private:
        static std::string _path;
};