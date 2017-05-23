#ifndef TJE_FRAMEWORK_2017_SCENE_H
#define TJE_FRAMEWORK_2017_SCENE_H

#include "entity.h"
#include "player.h"
class Scene {
    static Scene* scene;
    Scene();
    ~Scene();
public:
    Entity* root;
    Entity* background;
    Entity* grid;

    std::string name;

    static Scene* getScene();
    void addToRoot(Entity* e);
    void addBackground(Entity* e);
    void addGrid(Entity* e);
    void render(Camera* camera);
    void loadScene(const char* filename);
    void update(float elapsed_time);
};


#endif //TJE_FRAMEWORK_2017_SCENE_H
