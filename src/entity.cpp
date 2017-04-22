#include "entity.h"
#include <algorithm>    // std::find

Entity::Entity() {
    parent = NULL;
    model.rotateLocal(-M_PI_2/2, Vector3(0.0,0.0,1.0));
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

Matrix44 Entity::getGlobalModel() {
    if(parent)
        return model * parent->getGlobalModel();
    else return model;
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
    Shader* shader = Shader::Load(shaderDesc.vs,shaderDesc.fs);
    Matrix44 globalModel = getGlobalModel();
    Matrix44 mvp = globalModel * camera->viewprojection_matrix;
    shader->enable();
    shader->setMatrix44("u_model", model);
    shader->setMatrix44("u_mvp", mvp);
    shader->setTexture("u_texture", Texture::Load(texture));
    Mesh::Load(mesh)->render(GL_TRIANGLES, shader);
    shader->disable();

    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }
    
}

void EntityMesh::update(float elapsed_time){

}

void EntityMesh::followWithCamera(Camera* camera){
    Matrix44 globalModel = getGlobalModel();
    Vector3 entityPos = globalModel.getTranslationOnly();//Absolute position
    camera->eye = Vector3(entityPos.x-17,entityPos.y+5,entityPos.z);
    /*
    camera->lookAt(Vector3(entityPos.x-17,entityPos.y+5,entityPos.z),
                   entityPos,
                   Vector3(1.0,0.0,0.0));  //Hay que calcular como esta la esntity
    */
}