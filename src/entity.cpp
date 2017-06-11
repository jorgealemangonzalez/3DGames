#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find
#include <sstream>
#include "utils.h"
#include "controller.h"
#include "game.h"
#include "scene.h"

UID Entity::s_created = 1;
std::map<UID,Entity*> Entity::s_entities;
std::vector<UID> Entity::to_destroy;
std::vector<UID> EntityCollider::static_colliders;
std::vector<UID> EntityCollider::dynamic_colliders;

Entity::Entity() : uid(Entity::s_created++), parent(NULL) {
    stats.selected = false;
    save();
}
Entity::~Entity() {}

//Static methods

Entity* Entity::getEntity(UID uid) {
    auto it = Entity::s_entities.find(uid);
    if(it == Entity::s_entities.end())
        return NULL;
    return it->second;
}

void Entity::destroy_entities_to_destroy() {
    //TODO
}

std::vector<UID> Entity::entityPointed(Vector2 mouseDown, Vector2 mouseUp, int width, int height, Camera* camera){
    //Project all selectable entities into screen space and check if if are inside the region

    float up = (mouseDown.y > mouseUp.y ? mouseDown.y : mouseUp.y) + 5;
    float down = (mouseDown.y > mouseUp.y ? mouseUp.y : mouseDown.y) - 5;
    float right = (mouseDown.x > mouseUp.x ? mouseDown.x : mouseUp.x) + 5;
    float left = (mouseDown.x > mouseUp.x ? mouseUp.x : mouseDown.x) - 5;

    std::vector<UID> inside;

    for(auto &entry : Entity::s_entities){
        if(entry.second->stats.selectable){
            entry.second->stats.selected = false;
            Vector3 pos = camera->project(entry.second->getPosition(), width, height);
            if(EntityMesh* em = dynamic_cast<EntityMesh*>(entry.second)) {
                double meshRadius = Mesh::Load(em->mesh)->info.radius;
                double radius = camera->getProjectScale(entry.second->getPosition(), meshRadius)/1200.;

                if(up > pos.y-radius && down < pos.y+radius &&
                        right > pos.x-radius && left < pos.x+radius){
                    inside.push_back(entry.second->uid);
                    entry.second->stats.selected = true;
                }else if(up < pos.y+radius && down > pos.y-radius &&
                        right < pos.x+radius && left > pos.x-radius){
                    inside.push_back(entry.second->uid);
                    entry.second->stats.selected = true;
                }
            }else{
                if(pos.x >= left && pos.x <= right && pos.y >= down && pos.y <= up){
                    inside.push_back(entry.second->uid);
                    entry.second->stats.selected = true;
                }
            }
        }
    }
    return inside;
/*
    //Make a ray from the position of the eye of the camera
    //and get the uid of the entity that collides with it
    Vector3 pointingAt = camera->unproject(Vector3(mouseDown.x, mouseDown.y, 0), width, height);
    Vector3 direction = pointingAt - camera->eye;
    direction.normalize();

    UID closest_uid = 0;
    Vector3 collision;
    double minDistance = INFINITY;

    for(UID &entity_uid : EntityCollider::dynamic_colliders){
        EntityCollider* const entity = (EntityCollider*)Entity::getEntity(entity_uid);
        if(entity->testRayCollision(camera->eye, direction, INFINITY, collision)){
            double distance = camera->eye.distance(collision);
            if(distance < minDistance){
                minDistance = distance;
                closest_uid = entity_uid;
            }
        }
    }

    for(UID &entity_uid : EntityCollider::static_colliders){
        EntityCollider* const entity = (EntityCollider*)Entity::getEntity(entity_uid);
        if(entity->testRayCollision(camera->eye, direction, INFINITY, collision)){
            double distance = camera->eye.distance(collision);
            if(distance < minDistance){
                minDistance = distance;
                closest_uid = entity_uid;
            }
        }
    }

    return closest_uid;
*/
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

Vector3 Entity::getDirection() {
    return getGlobalModel().rotateVector(Vector3(0,0,-1));
}

void Entity::lookPosition(float seconds_elapsed, Vector3 toLook) {
    Vector3 to_target = toLook - getPosition();

    float distance = to_target.length();
    Vector3 looking = getDirection().normalize();
    to_target.normalize();
    float angle = acosf(looking.dot(to_target));
    Vector3 perpendicular = to_target.cross(looking).normalize();

    Matrix44 inv = getGlobalModel();
    inv.inverse();
    Vector3 perpendicularRotate = inv.rotateVector(perpendicular);
    float angleRotate = (angle > 0.03 ? angle * seconds_elapsed : angle);    //Angulo pequeño rota directamente
    if(angleRotate > 0)
        model.rotateLocal(angleRotate,perpendicularRotate);


    //TODO quit this
    Vector3 pos = getPosition();
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+perpendicular*100);
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+looking*100);
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+to_target*100);
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

void Entity::update(float seconds_elapsed){
    //std::cout<<"Entity_ "<< this->name <<"Health: "<<this->stats.hp<<"\n";

    if(stats.movable){
        //std::cout<<"UDATE ENTITY\n";
        // use elapsed_time, stats.vel and stats.front to move the entity
        if(stats.vel && stats.targetPos){
            //std::cout<<"UDATE ENTITY--------\n";

            Vector3 to_target = stats.targetPos - getPosition();
            to_target.normalize();
            Vector3 looking = getDirection().normalize();


            float angle = acosf(looking.dot(to_target));
            Vector3 perpendicular = to_target.cross(looking).normalize();

            Matrix44 inv = getGlobalModel();
            inv.inverse();
            Vector3 perpendicularRotate = inv.rotateVector(perpendicular);
            float angleRotate = (angle > 0.03 ? angle * seconds_elapsed : angle);    //Angulo pequeño rota directamente
            if(angleRotate > 0) {
                model.rotateLocal(angleRotate, perpendicularRotate);
            }

            float distance = (stats.targetPos - getPosition()).length();
            float velocity = stats.vel;
            if(distance < 100){         //parking velocity :')
                velocity = velocity* distance/100;
            }

            model.traslateLocal(0,0,-velocity * seconds_elapsed);     //TODO change this translate to some velocity vector

            if(distance < 10) {
                stats.vel = Vector3();
                stats.targetPos = Vector3();
            }
        }
    }
    if(stats.has_hp){
        if(stats.hp < 0)
            destroy();

    }
    if(stats.has_ttl){
        stats.ttl -= seconds_elapsed;
        if(stats.ttl < 0)
            destroy();
    }

    for(int i=0; i<children.size(); i++){
        children[i]->update(seconds_elapsed);
    }
}

void Entity::print(int depth) {
    for(int i=0; i<depth; i++) std::cout << "\t";
    std::cout << uid << "\n";

    for(int i=0; i<children.size(); i++){
        children[i]->print(depth+1);
    }
}
//================================================

EntitySpawner::EntitySpawner(): Entity() {

}

EntitySpawner::~EntitySpawner() {

}

void EntitySpawner::spawnEntity() {
    EntityCollider* newCollider = (EntityCollider*)Entity::getEntity(entitySpawned)->clone();
    Vector3 spawnPos = getGlobalModel().getTranslationOnly();
    newCollider->model.setTranslation(spawnPos.x,spawnPos.y,spawnPos.z);
    this->parent->addChild(newCollider);
    EntityCollider::registerCollider(newCollider);
    Game::instance->enemy->addControllableEntity(newCollider->uid); //TODO quit this hack
}

Entity* EntitySpawner::clone() {
    EntitySpawner* clon = new EntitySpawner();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntitySpawner::render(Camera *camera) {

}

void EntitySpawner::update(float elapsed_time) {
    lastSpawn += elapsed_time;
    if(spawnTime < lastSpawn){
        lastSpawn = 0.0;
        spawnEntity();
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
        shader->setVector3("camera_position", camera->eye);
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

//================================================

EntityCollider::EntityCollider(bool dynamic) : EntityMesh(), dynamic(dynamic) {}
EntityCollider::EntityCollider():EntityCollider(false){}
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
        double source_radius = 10.0;//TODO change this, make radius work

        for(int st = 0 ; st < static_colliders.size(); ++st){
            entityDest = (EntityCollider*)Entity::getEntity(static_colliders[st]);
            if(entityDest == NULL) {static_colliders.erase(static_colliders.begin() + i); --st; continue;}
            if(entityDest->testSphereCollision(dinamic_pos_source, source_radius, collision)) { //TODO change sphere collision for Mesh collision when object is to big
                entitySource->onCollision(entityDest);
                entityDest->onCollision(entitySource);
            }
        }

        for(int j = i+1 ; j < dynamic_colliders.size(); ++j){
            entityDest = (EntityCollider*)Entity::getEntity(dynamic_colliders[j]);
            if(entityDest == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + j); --j; continue;}
            if(entityDest->testSphereCollision(dinamic_pos_source, source_radius, collision)){
                entitySource->onCollision(entityDest);
                entityDest->onCollision(entitySource);
            }
        }
    }
    //std::cout<<"Time checkcollisions: "<<getTime()-now<<"\n";
}

bool EntityCollider::testRayCollision(Vector3 &origin, Vector3 &dir, float max_dist, Vector3 &collision_point){    //With ray
    CollisionModel3D* cm = Mesh::Load(mesh)->getCollisionModel();
    cm->setTransform(this->getGlobalModel().m);
    if(!cm->rayCollision(origin.v,dir.v,true,0,max_dist)){ //
        return false;
    }
    cm->getCollisionPoint(collision_point.v, false);    //Coordenadas de mundo
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
    if(_DEBUG_)
        std::cout<<this->mesh<<" collides with "<<withEntity->mesh<<std::endl;
}

void EntityCollider::onCollision(Bullet *withBullet) {
    if(_DEBUG_)
        std::cout<<this->mesh<<" collides with BULLET"<<std::endl;
    if(stats.has_hp)
        stats.hp -= 500;
}

//methods overwriten by derived classes
EntityCollider* EntityCollider::clone() {
    EntityCollider* clon = new EntityCollider();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

//==================================================

EntityFighter::EntityFighter(bool dynamic = false):EntityCollider(dynamic) {

}

EntityFighter::~EntityFighter() {

}

EntityFighter* EntityFighter::clone() {
    EntityFighter* clon = new EntityFighter();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntityFighter::shoot() {
    Vector3 actual_pos = getPosition();
    Vector3 dir= getDirection();
    dir.normalize();
    float radius = Mesh::Load(mesh)->info.radius + 4.0f;
    BulletManager::getManager()->createBullet(actual_pos + dir*radius,actual_pos + dir*radius,dir*300,100.0f,10.0f,uid,"No type yet");
    lastFireSec = 0;
}

void EntityFighter::update(float elapsed_time){
    Entity::update(elapsed_time);
    lastFireSec+=elapsed_time;
    if(!stats.targetPos)
    for(UID e : Game::instance->enemy->controllable_entities){
        Entity* focus = Entity::getEntity(e);
        if((focus->getPosition() - getPosition()).length() < 1000 ){ //TODO fire range variable
            lookPosition(elapsed_time,focus->getPosition());
            if(lastFireSec > 1.0/fireRate)
                shoot();
            break;
        }
    }
}