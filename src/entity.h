#ifndef TJE_FRAMEWORK_2017_ENTITY_H
#define TJE_FRAMEWORK_2017_ENTITY_H

#include <vector>

#include "constants.h"
#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "framework.h"
#include "camera.h"
#include "extra/coldet/src/coldet.h"
#include "BulletManager.h"
#include "GUI.h"

typedef struct{//NO PONER PUNTEROS EN ESTA ESTRUCTURA
    bool movable=false;         // Puede alterar su movimiento?
    bool has_hp=false;          // Puede destruirse a causa de hp <= 0
    bool has_ttl=false;         // Puede destruirse a cause de ttl <= 0
    bool selectable=false;      // Se puede seleccionar?
    bool selected=false;
    int hp=0;                   // Life remain
    int maxhp=0;                // Max life
    float ttl=0;                // Time remain
    std::string team="neutral"; // Ingame team: 't1', 't2', 'neutral'
    int vel=0;                  // Velocity intensity
    Vector3 targetPos;          // Target position of the entity
    UID followEntity = 0;
} Stats;
std::string sstats(Stats s);

class Entity{
public:
    static UID s_created; //Up to 4M entities
    static std::map<UID,Entity*> s_entities;
    static std::vector<UID> to_destroy;

    Entity();
    virtual ~Entity();

    UID uid;

    //Attributes
    std::string name;
    Matrix44 model;
    Entity* parent;
    std::vector<Entity*> children;

    //Game stats
    Stats stats;

    //Static methods
    static Entity* getEntity(UID uid);
    static void destroy_entities_to_destroy();
    static std::vector<UID> entityPointed(Vector2 mouseDown, Vector2 mouseUp, int width, int height, Camera* camera);

    //Entity methods
    void save();
    void addChild(Entity* ent);
    void removeChild(Entity* ent);
    void destroy();

    Matrix44 getGlobalModel();
    Vector3 getPosition();
    Vector3 getRotation();
    Vector3 getDirection();
    void lookPosition(float seconds_elapsed, Vector3 toLook);

    void followWithCamera(Camera* camera);

    //methods overwriten by derived classes
    virtual Entity* clone();
    virtual void render(Camera* camera);
    virtual void update(float elapsed_time);
    virtual void updateGUI();
    void updateStats(float elapsed_time);
    //debug
    virtual void print(int depth);
};

class EntityMesh : public Entity{   //ATENCION NO PONER PUNTEROS EN ESTA CLASE
public:
    std::string mesh;
    std::string texture;
    struct{
        std::string vs;
        std::string fs;
    } shaderDesc;
    Vector3 color;

    EntityMesh();
    virtual ~EntityMesh();

    //Entity methods
    std::string getMesh();
    std::string getTexture();
    std::string getVertexShader();
    std::string getPixelShader();
    Vector3 getColor();

    void setMesh(std::string mesh);
    void setTexture(std::string texture);
    void setVertexShader(std::string vs);
    void setPixelShader(std::string fs);
    void setColor(Vector3 color);

    //methods overwriten by derived classes
    virtual EntityMesh* clone();
    virtual void render(Camera* camera);
    virtual void updateGUI();
};


class EntityCollider : public EntityMesh{ //ATENCION NO PONER PUNTEROS EN ESTA CLASE
public:
    static std::vector<UID> static_colliders;   // NO se mueven, así que no hay colisiones entre ellas
    static std::vector<UID> dynamic_colliders;  // Pueden colisionar entre ellas y con static_colliders

    bool dynamic;

    EntityCollider(bool dynamic);
    EntityCollider();
    virtual ~EntityCollider();

    //Static methods
    static void registerCollider(EntityCollider* e);
    static void checkCollisions();

    bool testRayCollision(Vector3 &origin, Vector3 &dir, float max_dist, Vector3 &collision_point);
    bool testSphereCollision(Vector3 &origin, float radius, Vector3 &collision_point);
    bool testMeshCollision(EntityMesh *testMesh);
    void onCollision(EntityCollider* withEntity);
    void onCollision(Bullet *withBullet);

    //methods overwriten by derived classes
    virtual EntityCollider* clone();
};

class EntityFighter : public EntityCollider{ //NO PONER PUNTEROS EN ESTA CLASE
public:
    float fireRate = 3.0f; //bullets per second
    float lastFireSec = 1000; //Seconds since last fire
    EntityFighter(bool dynamic = false);
    virtual ~EntityFighter();

    void shoot();
    EntityFighter* clone();
    void update(float elapsed_time);
};

class EntitySpawner : public Entity{ //ATENCION NO PONER PUNTEROS EN ESTA CLASE
public:
    UID entitySpawned;
    Stats statsSpawned;
    float spawnTime;
    float lastSpawn;

    EntitySpawner();
    virtual ~EntitySpawner();

    void spawnEntity();
    //Inherit from entity
    Entity* clone();
    void render(Camera* camera);
    void update(float elapsed_time);

};

#endif //TJE_FRAMEWORK_2017_ENTITY_H
