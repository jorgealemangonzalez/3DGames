#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find

Entity::Entity() {
    parent = NULL;
}

Entity::~Entity() {

}

void Entity::addChild(Entity* ent){
    if(ent->parent){
        std::cout<<"This entity already has parent"<<std::endl;
        return;
    }
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

Vector3 Entity::getRotation() {
    return getGlobalModel().getRotationOnly() * Vector3();
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
EntityMesh::~EntityMesh(){

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
        if(texture != "")
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

EntityCollider::EntityCollider(){

}

EntityCollider::~EntityCollider(){

}

bool EntityCollider::testCollision(Vector3& origin, Vector3& dir, float max_dist, Vector3& collision_point){
    Mesh* m = Mesh::Load(mesh);
    if(m->getCollisionModel()->rayCollision(origin.v,dir.v,true)== false){ //
        return false;
    }
    m->getCollisionModel()->getCollisionPoint(collision_point.v, true);    //Coordenadas de objeto o de mundo ?
    return true;
}

void EntityCollider::setTransform(){
    Mesh::Load(mesh)->getCollisionModel()->setTransform(model.m); //Costoso !
}

