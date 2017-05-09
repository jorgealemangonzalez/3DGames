#ifndef TJE_FRAMEWORK_2017_SCENE_H
#define TJE_FRAMEWORK_2017_SCENE_H

#include "entity.h"

class Scene {
    static Scene* scene;
    Scene();
    ~Scene();
public:
    Entity* root;
    static Scene* getScene();
    static void addToRoot(Entity* e);
    static void render(Camera* camera);
    static void update(float elapsed_time);
};


#endif //TJE_FRAMEWORK_2017_SCENE_H
