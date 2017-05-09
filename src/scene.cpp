#include "scene.h"

Scene* Scene::scene = NULL;
Scene::Scene(){
    root = new Entity();
}
Scene::~Scene(){

}

Scene* Scene::getScene() {
    if(scene == NULL){
        scene = new Scene();
    }
    return scene;
}

void Scene::addToRoot(Entity *e) {
    Scene::getScene()->root->addChild(e);
}

void Scene::render(Camera* camera) {
    Scene::getScene()->root->render(camera);
}

void Scene::update(float elapsed_time) {
    Scene::getScene()->root->update(elapsed_time);
}