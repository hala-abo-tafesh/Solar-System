#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Sphere {
public:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float radius;
    unsigned int sectorCount, stackCount;

    Sphere(float r = 1.0f, unsigned int sectors = 36, unsigned int stacks = 18);
    void buildSphere();
    void Draw();
};

#endif // SPHERE_H
