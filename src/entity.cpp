#include "entity.h"

Entity::Entity() {

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
    mesh = NULL;
    texture = NULL;
    shader = NULL;

    default_shader = Shader::Load("../data/shaders/texture.vs","../data/shaders/texture.fs");
}

void EntityMesh::render(Camera* camera){
    Shader* shader = EntityMesh::default_shader;

    Matrix44 mvp = model * camera->viewprojection_matrix;
    shader->enable();
    shader->setMatrix44("u_model", model);
    shader->setMatrix44("u_mvp", mvp);
    shader->setTexture("u_texture", Texture::Load("../data/meshes/spitfire/spitfire_color_spec.tga"));
    mesh->render(GL_TRIANGLES, shader);
    shader->disable();

    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }
    
}

void EntityMesh::update(float elapsed_time){

}