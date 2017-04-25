#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find

Entity::Entity() {
    parent = NULL;
}

Entity::~Entity() {

}

void Entity::addChild(Entity* ent){
    ent->parent = this;
    children.push_back(ent);
}

void Entity::removeChild(Entity* entity){
    entity->parent = NULL;
    auto it = std::find(children.begin(), children.end(), entity);
    if(it==children.end())
        return;
    children.erase(it);
    entity->model = entity->model * getGlobalModel();
}

void Entity::destroy() {
    //to_destroy.push_back(this);
}

Matrix44 Entity::getGlobalModel() {
    if(parent)
        return model * parent->getGlobalModel();
    else return model;
}

Vector3 Entity::getPosition() {
    return getGlobalModel() * Vector3();
}

void Entity::render(Camera* camera){
    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }
}

void Entity::update(float elapsed_time){
}

EntityMesh::EntityMesh(){
    mesh = "";
    texture = "";
    shaderDesc.vs = "texture.vs";
    shaderDesc.fs = "texture.fs";
}

void EntityMesh::render(Camera* camera){
    Matrix44 globalModel = getGlobalModel();
    Matrix44 mvp = globalModel * camera->viewprojection_matrix;
    Mesh* m = Mesh::Load(mesh);
    if(camera->testSphereInFrustum(getPosition(), m->info.radius)){
        Shader* shader = Shader::Load(shaderDesc.vs,shaderDesc.fs);
        shader->enable();
        shader->setMatrix44("u_model", model);
        shader->setMatrix44("u_mvp", mvp);
        shader->setTexture("u_texture", Texture::Load(texture));
        m->render(GL_TRIANGLES, shader);
        shader->disable();
    }

    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }

}

void EntityMesh::update(float elapsed_time){

}

void EntityMesh::followWithCamera(Camera* camera){
    Vector3 pos = getPosition();
    camera->lookAt(pos + Vector3(0.f, 10.f, 20.f), pos, Vector3(0.f, 1.f, 0.f));
}