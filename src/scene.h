#ifndef TJE_FRAMEWORK_2017_SCENE_H
#define TJE_FRAMEWORK_2017_SCENE_H

#include "includes.h"
#include "entity.h"
#include "player.h"
class Scene {
    static Scene* scene;
    Scene();
public:
    ~Scene();
    Entity* root;
    Entity* background;

    Entity* root_background;

    std::string name;

    static Scene* getScene();
    void addToRoot(Entity* e);
    void addBackground(Entity* e);
    void render(Camera* camera);
    void loadScene(const char* filename);
    void update(float elapsed_time);
    void updateGUI();
};


#endif //TJE_FRAMEWORK_2017_SCENE_H
