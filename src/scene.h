#ifndef TJE_FRAMEWORK_2017_SCENE_H
#define TJE_FRAMEWORK_2017_SCENE_H

#include "includes.h"
#include "entity.h"
#include "player.h"
class Scene {
    static Scene* scene;
    Scene();
    ~Scene();
public:
    //bool debugMode;
    Entity* root;
    Entity* background;
    Entity* grid;

    Entity* root_background;

    Camera* camera2d;
    Mesh* debugPointsMesh;
    Mesh* debugLinesMesh;

    std::string name;

    static Scene* getScene();
    void addToRoot(Entity* e);
    void addBackground(Entity* e);
    void addGrid(Entity* e);
    void render(Camera* camera);
    void loadScene(const char* filename);
    void update(float elapsed_time);
    void addDebugPoint(Vector3 pos1);
    void addDebugPoint(Vector3 pos1, Vector4 color);
    void addDebugLine(Vector3 pos1, Vector3 pos2);
    void addDebugLine(Vector3 pos1, Vector3 pos2, Vector4 color);
};


#endif //TJE_FRAMEWORK_2017_SCENE_H
