#pragma once

#include <assimp/scene.h>

#include <functional>
#include <string>
#include <vector>

class ResourceProcessor {
  public:
    virtual void processResource(std::string name, const aiScene *resource) = 0;
};

class Importer {
  public:
    static bool LoadResource(std::string name, ResourceProcessor *p, std::string &error); // load the entire resource
    static void setPath(std::string path);
    static unsigned char *LoadTexture(std::string resourcePath, int &width, int &height, int &nrChannels);
    static void freeResource(unsigned char *data); // only for LoadTexture
    static std::string getFilePath(std::string name);

  private:
    static std::string _path;
};