#ifndef TJE_FRAMEWORK_2017_ENTITY_H
#define TJE_FRAMEWORK_2017_ENTITY_H

#include <vector>

#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "framework.h"
#include "camera.h"
#include "extra/coldet/src/coldet.h"

class Entity{
public:
    static unsigned int s_created; //Up to 4M entities
    static std::map<unsigned int,Entity*> s_entities;

    static Entity* getEntity(unsigned int uid);

    unsigned int uid;

    Entity();
    ~Entity();

    static std::vector<Entity*> to_destroy;
    static void destroy_entities_to_destroy();

    //some attributes
    std::string name;
    Matrix44 model;
    Entity* parent;
    std::vector<Entity*> children;

    //Entity methods
    void addChild(Entity* ent);
    void removeChild(Entity* ent);
    void destroy();

    Matrix44 getGlobalModel();
    Vector3 getPosition();
    Vector3 getRotation();

    //methods overwriten by derived classes
    virtual void render(Camera* camera);
    virtual void update(float elapsed_time);
};

class EntityMesh : public Entity{   //ATENCION NO PONER PUNTEROS EN ESTA CLASE
protected:
    std::string mesh;
public:
    EntityMesh();
    ~EntityMesh();
    std::string texture;
    struct{
        std::string vs;
        std::string fs;
    } shaderDesc;
    Vector3 color;

    std::string getMesh();
    void setMesh(std::string mesh);

    void render(Camera* camera);
    void update(float elapsed_time);

    void followWithCamera(Camera* camera);
};


class EntityCollider : public EntityMesh{ //ATENCION NO PONER PUNTEROS EN ESTA CLASE

public:
    EntityCollider();
    ~EntityCollider();

    bool testCollision(Vector3& origin, Vector3& dir, float max_dist, Vector3& collision_point);
    bool testCollision(Vector3& origin, float radius, Vector3& collision_point);
    void setTransform();
    void onCollision(EntityCollider* withEntity);
};

#endif //TJE_FRAMEWORK_2017_ENTITY_H
