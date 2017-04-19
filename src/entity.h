//
// Created by jorgealemangonzalez on 19/04/17.
//

#ifndef TJE_FRAMEWORK_2017_ENTITY_H
#define TJE_FRAMEWORK_2017_ENTITY_H

#include <vector>
#include "texture.h"
#include "mesh.h"
#include "framework.h"
#include "camera.h"

class Entity{
public:
    Entity();
    ~Entity();

    //some attributes
    std::string name;
    Matrix44 model;
    Entity* parent;
    std::vector<Entity*> children;
    //Entity methods
    void addChild(Entity* ent);
    void removeChild(Entity* ent);

    //methods overwriten by derived classes
    virtual void render(Camera* camera);
    virtual void update(float elapsed_time);

    Vector3 getPosition();

};

class EntityMesh : public Entity{
public:
    EntityMesh();
    ~EntityMesh();

    std::string mesh;
    std::string texture;
    std::string shader;
    Vector3 color;
    Shader* default_shader = NULL;

    void render(Camera* camera);
    void update();
};
#endif //TJE_FRAMEWORK_2017_ENTITY_H
