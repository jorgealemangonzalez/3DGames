#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find
#include <sstream>

UID Entity::s_created = 1;
std::map<UID,Entity*> Entity::s_entities;
std::vector<UID> Entity::to_destroy;
std::vector<UID> EntityCollider::static_colliders;
std::vector<UID> EntityCollider::dynamic_colliders;

Entity::Entity() : uid(Entity::s_created++), parent(NULL){ save(); }
Entity::~Entity() {}

//Static methods

Entity* Entity::getEntity(UID uid) {
    auto it = Entity::s_entities.find(uid);
    if(it == Entity::s_entities.end())
        return NULL;
    return it->second;
}

void Entity::destroy_entities_to_destroy() {

}


//Entity methods

void Entity::save() {
    Entity::s_entities[uid] = this;
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
    Entity* e = this;
    to_destroy.push_back(e->uid);
}

Matrix44 Entity::getGlobalModel() {
    if(parent)
        return model * parent->getGlobalModel();
    else return model;
}

Vector3 Entity::getPosition() {
    Matrix44 m = getGlobalModel();
    return getGlobalModel() * Vector3();
}

Vector3 Entity::getRotation() {
    return getGlobalModel().getRotationOnly() * Vector3();
}

void Entity::followWithCamera(Camera* camera){
    Vector3 pos = getPosition();
    camera->lookAt(pos + Vector3(0.f, 10.f, 20.f), pos, Vector3(0.f, 1.f, 0.f));
}

//methods overwriten by derived classes
Entity* Entity::clone() {
    Entity* clon = new Entity();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void Entity::render(Camera* camera){
    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }
}

void Entity::update(float elapsed_time){
    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}

//================================================

EntityMesh::EntityMesh() : Entity() {
    mesh = "";
    texture = "";
    shaderDesc.vs = "texture.vs";
    shaderDesc.fs = "texture.fs";
}

EntityMesh::~EntityMesh(){}

//Static methods
//Entity methods

std::string EntityMesh::getMesh() { return mesh; }
std::string EntityMesh::getTexture(){ return texture; }
std::string EntityMesh::getVertexShader(){ return shaderDesc.vs; }
std::string EntityMesh::getPixelShader() {return shaderDesc.fs; }
Vector3 EntityMesh::getColor() { return color; }

void EntityMesh::setMesh(std::string mesh) { this->mesh = mesh; }
void EntityMesh::setTexture(std::string texture){ this->texture = texture; }
void EntityMesh::setVertexShader(std::string vs){ this->shaderDesc.vs = vs; }
void EntityMesh::setPixelShader(std::string fs){ this->shaderDesc.fs = fs; }
void EntityMesh::setColor(Vector3 color) {this->color = color; }

//methods overwriten by derived classes
EntityMesh* EntityMesh::clone() {
    EntityMesh* clon = new EntityMesh();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntityMesh::render(Camera* camera){
    //std::cout << mesh << "\n";

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

    // Entity::render(camera); //Se podra hacer asi?
    for(int i=0; i<children.size(); i++){
        children[i]->render(camera);
    }

}

void EntityMesh::update(float elapsed_time){
    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}

//================================================

EntityCollider::EntityCollider() : EntityMesh(), dynamic(false) {}
EntityCollider::EntityCollider(bool dynamic) : EntityMesh(), dynamic(dynamic) {}
EntityCollider::~EntityCollider(){}

//Static methods
void EntityCollider::registerCollider(EntityCollider* e) {
    if(e->dynamic){
        EntityCollider::dynamic_colliders.push_back(e->uid);
    }else{
        EntityCollider::static_colliders.push_back(e->uid);
    }
}

void EntityCollider::checkCollisions() {
    //Test every possible collision
    Vector3 collision;
    EntityCollider *e_1, *e_2;
    std::stringstream ss;
    for(int i = 0 ; i < dynamic_colliders.size(); ++i){
        e_1 = (EntityCollider*)Entity::getEntity(dynamic_colliders[i]);
        if(e_1 == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + i); --i; continue;}
        e_1->setTransform();
        Vector3 dinamic_pos = e_1->getGlobalModel().getTranslationOnly();

        for(int st = 0 ; st < static_colliders.size(); ++st){
            e_2 = (EntityCollider*)Entity::getEntity(static_colliders[st]);
            if(e_2 == NULL) {static_colliders.erase(static_colliders.begin() + i); --st; continue;}
            if(e_2->testCollision(dinamic_pos,20.0f, collision)) {
                e_1->onCollision(e_2);
                e_2->onCollision(e_1);
                ss << e_1->name << " (" << e_1->uid << ") <-> " << e_2->name << " (" << e_2->uid << ")\n";
            }
        }

        for(int j = i+1 ; j < dynamic_colliders.size(); ++j){
            e_2 = (EntityCollider*)Entity::getEntity(dynamic_colliders[j]);
            if(e_2 == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + j); --j; continue;}
            if(e_2->testCollision(dinamic_pos,20.0f, collision)){
                e_1->onCollision(e_2);
                e_2->onCollision(e_1);
                ss << e_1->name << " (" << e_1->uid << ") <-> " << e_2->name << " (" << e_2->uid << ")\n";
            }
        }
    }
}

bool EntityCollider::testCollision(Vector3& origin, Vector3& dir, float max_dist, Vector3& collision_point){    //With ray
    Mesh* m = Mesh::Load(mesh);
    if(!m->getCollisionModel()->rayCollision(origin.v,dir.v,true)){ //
        return false;
    }
    m->getCollisionModel()->getCollisionPoint(collision_point.v, true);    //Coordenadas de objeto o de mundo ?
    if(origin.distance(collision_point) >= max_dist)
        return false;

    return true;
}

bool EntityCollider::testCollision(Vector3& origin, float radius, Vector3& collision_point){    //With Sphere
    Mesh* m = Mesh::Load(mesh);
    if(!m->getCollisionModel()->sphereCollision(origin.v,radius)){ //
        return false;
    }
    m->getCollisionModel()->getCollisionPoint(collision_point.v, true);    //Coordenadas de objeto o de mundo ?
    return true;
}

bool EntityCollider::testCollision(EntityMesh *testMesh) {
    Mesh* sourceMesh = Mesh::Load(mesh);
    Mesh* destMesh = Mesh::Load(testMesh->mesh);

    bool collide = sourceMesh->getCollisionModel()->collision(
            destMesh->getCollisionModel(), -1 , 0, testMesh->getGlobalModel().m);
    return collide;
}

void EntityCollider::setTransform(){
    Mesh::Load(mesh)->getCollisionModel()->setTransform(model.m); //Costoso !
}

void EntityCollider::onCollision(EntityCollider *withEntity) {
    //std::cout<<this->mesh<<" collides with "<<withEntity->mesh<<std::endl;
}

void EntityCollider::onCollision(Bullet *withBullet) {
    std::cout<<this->mesh<<" collides with BULLET"<<std::endl;
}

//methods overwriten by derived classes
EntityCollider* EntityCollider::clone() {
    EntityCollider* clon = new EntityCollider();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntityCollider::update(float elapsed_time) {
    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}
