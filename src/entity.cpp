#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find
#include <sstream>
#include "utils.h"

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

Vector3 Entity::getDirection() {    //TODO use this everywhere
    return getGlobalModel().rotateVector(Vector3(0,0,-1));
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

    //TODO Entity::render(camera); //Se podra hacer asi?
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
    auto now = getTime();

    Vector3 collision;
    EntityCollider *entitySource, *entityDest;
    std::stringstream ss;
    for(int i = 0 ; i < dynamic_colliders.size(); ++i){
        entitySource = (EntityCollider*)Entity::getEntity(dynamic_colliders[i]);
        if(entitySource == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + i); --i; continue;}
        //entitySource->setTransform(); //Ahora mismo solo se
        Vector3 dinamic_pos_source = entitySource->getGlobalModel().getTranslationOnly();
        double source_radius = Mesh::Load(entitySource->mesh)->info.radius;

        for(int st = 0 ; st < static_colliders.size(); ++st){
            entityDest = (EntityCollider*)Entity::getEntity(static_colliders[st]);
            if(entityDest == NULL) {static_colliders.erase(static_colliders.begin() + i); --st; continue;}
            if(entityDest->testSphereCollision(dinamic_pos_source, source_radius, collision)) {
                entitySource->onCollision(entityDest);
                entityDest->onCollision(entitySource);
                ss << entitySource->name << " (" << entitySource->uid << ") <-> " << entityDest->name << " (" << entityDest->uid << ")\n";
            }
        }

        for(int j = i+1 ; j < dynamic_colliders.size(); ++j){
            entityDest = (EntityCollider*)Entity::getEntity(dynamic_colliders[j]);
            if(entityDest == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + j); --j; continue;}
            if(entityDest->testSphereCollision(dinamic_pos_source, source_radius, collision)){
                entitySource->onCollision(entityDest);
                entityDest->onCollision(entitySource);
                ss << entitySource->name << " (" << entitySource->uid << ") <-> " << entityDest->name << " (" << entityDest->uid << ")\n";
            }
        }
    }
    std::cout<<"Time checkcollisions: "<<getTime()-now<<"\n";
}

bool EntityCollider::testRayCollision(Vector3 &origin, Vector3 &dir, float max_dist, Vector3 &collision_point){    //With ray
    CollisionModel3D* cm = Mesh::Load(mesh)->getCollisionModel();
    cm->setTransform(this->getGlobalModel().m);
    if(!cm->rayCollision(origin.v,dir.v,true,0,max_dist)){ //
        return false;
    }
    cm->getCollisionPoint(collision_point.v, true);    //Coordenadas de objeto o de mundo ?
    return true;
}

bool EntityCollider::testSphereCollision(Vector3 &origin, float radius, Vector3 &collision_point){    //With Sphere
    CollisionModel3D* cm = Mesh::Load(mesh)->getCollisionModel();
    cm->setTransform(this->getGlobalModel().m);
    if(!cm->sphereCollision(origin.v,radius)){ //
        return false;
    }
    cm->getCollisionPoint(collision_point.v, true);    //Coordenadas de objeto o de mundo ?
    return true;
}

bool EntityCollider::testMeshCollision(EntityMesh *testMesh) {
    CollisionModel3D* sourceCM = Mesh::Load(mesh)->getCollisionModel();
    CollisionModel3D* destCM = Mesh::Load(testMesh->mesh)->getCollisionModel();
    sourceCM->setTransform(this->getGlobalModel().m);
    destCM->setTransform(testMesh->getGlobalModel().m);

    bool collide = sourceCM->collision(
            destCM, -1 , 0, testMesh->getGlobalModel().m);
    return collide;
}

void EntityCollider::onCollision(EntityCollider *withEntity) {

    std::cout<<this->mesh<<" collides with "<<withEntity->mesh<<std::endl;
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
