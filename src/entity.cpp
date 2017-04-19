#include "entity.h"

void Entity::addChild(Entity* ent){
    children.push_back(ent);
}

void Entity::removeChild(Entity* ent){
    children.erase(ent);
}

void Entity::render(Camera* camera){

}

void Entity::update(float elapsed_time){

}

EntityMesh::EntityMesh(){
}

void EntityMesh::render(Camera* camera){
    Shader* shader = EntityMesh::default_shader;

    Matrix44 mvp = model * camera->viewprojection_matrix;
    shader->enable();
    shader->setMatrix44("u_model", model);
    shader->setMatrix44("u_mvp", mvp);
    
}

void EntityMesh::update(){

}