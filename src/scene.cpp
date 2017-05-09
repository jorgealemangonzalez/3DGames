#include "scene.h"

Scene* Scene::scene = NULL;
Scene::Scene(){
    root = new Entity();
    background = NULL;
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

void Scene::addBackground(Entity *e) {
    Scene::getScene()->background = e;
}

void Scene::render(Camera* camera) {
    if(background != NULL){
        glDisable(GL_DEPTH_TEST);
        Vector3 cubemapcenter = camera->eye;
        background->model.setIdentity().setTranslation(cubemapcenter.x, cubemapcenter.y, cubemapcenter.z);
        background->render(camera);
        glEnable(GL_DEPTH_TEST);
    }
    root->render(camera);
}

void Scene::update(float elapsed_time) {
    Scene::getScene()->root->update(elapsed_time);
}