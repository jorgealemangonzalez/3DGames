#include "entity.h"
#include "mesh.h"
#include <algorithm>    // std::find
#include <sstream>
#include "utils.h"
#include "controller.h"
#include "game.h"
#include "scene.h"
#include "MusicManager.h"

std::string sstats(Stats s){
    std::stringstream ss;

    ss << "HP: " << (s.has_hp ? "true " : "false ") << s.hp << "\n";

    ss << "TTL: " << (s.has_ttl ? "true " : "false ") << s.ttl << "\n";

    ss << "Select: " << (s.selectable ? "true " : "false ") << (s.selected ? "true" : "false") << "\n";

    ss << "Team: " << s.team << "\n";

    ss << "Movable: " << (s.movable ? "true " : "false ") << "vel: " << s.vel << " obj " << s.targetPos << "\n";
    return ss.str();
}

UID Entity::s_created = 1;
std::map<UID,Entity*> Entity::s_entities;
std::vector<UID> Entity::to_destroy;
std::vector<UID> EntityCollider::static_colliders;
std::vector<UID> EntityCollider::dynamic_colliders;

std::vector<Entity*> Entity::getAndCleanEntityVector(std::vector<UID> &uids) {
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = uids.begin(); it != uids.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        uids.erase(it);

    return entities;
}

Entity::Entity() : uid(Entity::s_created++), parent(NULL) {
    save();
}

Entity::~Entity() { //destructors are called automatically in the reverse order of construction. (Base classes last).
    //Remove from s_entities
    auto it = s_entities.find(this->uid);
    if(it != s_entities.end()){
        s_entities.erase(it);
    }
    std::cout<<"DELETE ENTITY: "<<this->name<<"\n";
    //Play sound enemy down if enemy
    if(stats.team == ENEMY_TEAM && Game::instance->gameState == PLAYING){
        MusicManager::playEnemyDown(getPosition());
    }
}

//Static methods

Entity* Entity::getEntity(UID uid) {
    auto it = Entity::s_entities.find(uid);
    if(it == Entity::s_entities.end())
        return NULL;
    return it->second;
}

void Entity::destroy_entities_to_destroy() {
    // Destroy all entities in the list to_destroy (the destructor should destroy their children)
    Game* g = Game::instance;
    for( UID uid : to_destroy){
        Entity* entity = s_entities[uid];
        if(entity == NULL)
            continue;
        if(entity->stats.mantainAlive) {
            Player* p = g->getTeamPlayer(entity->stats.team);
            auto it = std::find(p->maintainAliveEntities.begin(),p->maintainAliveEntities.end(),entity->uid);
            if(it != p->maintainAliveEntities.end()){
                std::cout<<"MANTAIN ALIVE ENTITY DESTROYED !!!\n";
                p->maintainAliveEntities.erase(it);
            }

            if(!p->maintainAliveEntities.size()){
                g->gameState = GAME_OVER;
                if(entity->stats.team == HUMAN_TEAM)
                    g->humanWins = false;
                else
                    g->humanWins = true;
            }

        }
        if(entity != NULL)
            delete entity;
    }
    to_destroy = std::vector<UID>();
}

void Entity::destroy_all() {
    while(s_entities.size() > 0){
        Entity *e = (*s_entities.begin()).second;
        delete e;
    }
    s_created = 0;
}

std::vector<UID> Entity::entityPointed(Vector2 mouseDown, Vector2 mouseUp, int width, int height, Camera* camera, std::string team){
    //Project all selectable entities into screen space and check if if are inside the region

    float up = (mouseDown.y > mouseUp.y ? mouseDown.y : mouseUp.y) + 5;
    float down = (mouseDown.y > mouseUp.y ? mouseUp.y : mouseDown.y) - 5;
    float right = (mouseDown.x > mouseUp.x ? mouseDown.x : mouseUp.x) + 5;
    float left = (mouseDown.x > mouseUp.x ? mouseUp.x : mouseDown.x) - 5;

    std::vector<UID> inside;

    for(auto &entry : Entity::s_entities){
        {
            if(entry.second->stats.team == NO_TEAM || entry.second->stats.isTemplate) continue;

            entry.second->stats.selected = false;
            Vector3 pos = camera->project(entry.second->getPosition(), width, height);
            if(EntityMesh* em = dynamic_cast<EntityMesh*>(entry.second)) {
                double meshRadius = Mesh::Load(em->mesh)->info.radius;
                double radius = camera->getProjectScale(entry.second->getPosition(), meshRadius)*2;

                if(up > pos.y-radius && down < pos.y+radius &&
                        right > pos.x-radius && left < pos.x+radius){
                    if(team == ANY_TEAM || team == entry.second->stats.team) {
                        inside.push_back(entry.second->uid);
                        if(entry.second->stats.selectable)
                            entry.second->stats.selected = true;
                    }
                }else if(up < pos.y+radius && down > pos.y-radius &&
                        right < pos.x+radius && left > pos.x-radius){
                    if(team == ANY_TEAM || team == entry.second->stats.team) {
                        inside.push_back(entry.second->uid);
                        if(entry.second->stats.selectable)
                            entry.second->stats.selected = true;
                    }
                }
            }else{
                if(pos.x >= left && pos.x <= right && pos.y >= down && pos.y <= up){
                    if(team == ANY_TEAM || team == entry.second->stats.team) {
                        inside.push_back(entry.second->uid);
                        if(entry.second->stats.selectable)
                            entry.second->stats.selected = true;
                    }
                }
            }
        }
    }
    return inside;
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
}

void Entity::destroy() {
    if(parent)
        parent->removeChild(this);
    for(Entity* child: children){
        child->destroy();
    }
    to_destroy.push_back(uid);
}

Matrix44 Entity::getGlobalModel() {
    if(parent != NULL)
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

    Vector3 looking = getDirection().normalize();
    to_target.normalize();
    float dotres = looking.dot(to_target);

    if(dotres > 0.999999)   //Parallel, same direction
        return;

    Vector3 perpendicular;
    if(dotres < -0.99999){ //Parallel, oposite directions
        perpendicular = to_target.cross(Vector3(looking.y, looking.z, looking.y)).normalize();
    }else{
        perpendicular = to_target.cross(looking).normalize();
    }

    float angle = acosf(dotres);

    Matrix44 inv = getGlobalModel();
    inv.inverse();
    Vector3 perpendicularRotate = inv.rotateVector(perpendicular);
    float angleRotate = (angle > 0.00001 ? angle * seconds_elapsed*3 : angle);    //Angulo pequeño rota directamente
    if(angleRotate > 0)
        model.rotateLocal(angleRotate,perpendicularRotate);


    //TODO quit this: Debug lines
    if(debugMode){
        GUI* gui = GUI::getGUI();
        Vector3 pos = getPosition();
        gui->addLine(pos, pos+perpendicular*100, Vector4(1,1,1,1), false, true);
        gui->addLine(pos, pos+looking*100, Vector4(1,1,1,1), false, true);
        gui->addLine(pos, pos+to_target*100, Vector4(1,1,1,1), false, true);
    }
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

void Entity::updateStatsAndEntityActions(float elapsed_time) {

    if(stats.movable){
        Vector3 pos = getPosition();

            //If it has a velocity and a target position try to move to that point
        if(stats.vel && stats.targetPos) {

            Vector3 to_target = stats.targetPos - pos;
            to_target.normalize();
            Vector3 looking = getDirection().normalize();


            float angle = acosf(looking.dot(to_target));
            Vector3 perpendicular = to_target.cross(looking).normalize();

            Matrix44 inv = getGlobalModel();
            inv.inverse();
            Vector3 perpendicularRotate = inv.rotateVector(perpendicular);
            float angleRotate = (angle > 0.03 ? angle * elapsed_time * (stats.vel/100) : angle);    //Angulo pequeño rota directamente
            if (angleRotate > 0) {
                model.rotateLocal(angleRotate, perpendicularRotate);
            }

            float distance = (stats.targetPos - pos).length();

            float velocity = stats.vel;
            if (velocity && distance < 100) {         //parking velocity :')
                velocity = (distance/100)* velocity;
            }
            /*if (debugMode)
                std::cout << "GRAVITY:: " << stats.gravity << std::endl;
            */
            stats.gravity *= elapsed_time;
            Vector3 vel = model.rotateVector(Vector3(0, 0, -velocity * elapsed_time));
            Vector3 added = vel + stats.gravity;
            added.normalize();
            added *= (velocity * elapsed_time);
            model.traslate(added.x, added.y, added.z);

            if (distance < 10 || (distance < stats.range*.4 && stats.followEntity)) {
                stats.vel = 0;
                stats.targetPos = Vector3();
            }
        }
    }
    if(stats.has_hp){
        if(stats.hp < 0)
            destroy();

    }
    if(stats.has_ttl){
        stats.ttl -= elapsed_time;
        if(stats.ttl < 0)
            destroy();
    }

    if(stats.gravity && !(stats.vel && stats.targetPos)){   //If we have already update the movent depending on gravity dont do it again
        stats.gravity *= elapsed_time;
        model.traslate(stats.gravity.x,stats.gravity.y,stats.gravity.z);
        stats.gravity = Vector3();
    }
}

void Entity::update(float elapsed_time){
    updateStatsAndEntityActions(elapsed_time);

    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}

void Entity::updateGUI() {
    for(int i=0; i<children.size(); i++){
        children[i]->updateGUI();
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

EntitySpawner::~EntitySpawner(){
}

void EntitySpawner::spawnEntity() {
    EntityCollider* newCollider = (EntityCollider*)Entity::getEntity(entitySpawned)->clone();
    Vector3 spawnPos = getGlobalModel().getTranslationOnly();

    double min_distance = Mesh::Load(newCollider->mesh)->info.radius * 2;
    bool near_entities = false;
    for(UID id : EntityCollider::dynamic_colliders){    //No hacen falta los estaticos porque ya se da por supuesto que el spawner esta bien puesto
        Entity* entity = Entity::getEntity(id);
        if(entity != NULL) {
            double d = (entity->getPosition() - spawnPos).length();
            if (d < min_distance)
                near_entities = true;
        }
    }

    if(near_entities){
        delete newCollider;
        return;
    }else{
        newCollider->save();    //Add to s_entities
        newCollider->stats = this->statsSpawned;
        Game::instance->getTeamPlayer(newCollider->stats.team)->addControllableEntity(newCollider->uid);

        newCollider->model.setTranslation(spawnPos.x,spawnPos.y,spawnPos.z);
        this->parent->addChild(newCollider);
        EntityCollider::registerCollider(newCollider);
    }
}

Entity* EntitySpawner::clone() {
    EntitySpawner* clon = new EntitySpawner();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntitySpawner::update(float elapsed_time) {
    updateStatsAndEntityActions(elapsed_time);
    lastSpawn += elapsed_time;
    if(spawnTime < lastSpawn){
        lastSpawn = 0.0;
        spawnEntity();
    }

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

EntityMesh::~EntityMesh(){

}

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

void EntityMesh::unitGUI() {
    Game* game = Game::instance;
    Camera* camera = game->camera;
    Vector4 color = GUI::getColor(stats.team, stats.selected);
    GUI* gui = GUI::getGUI();

    Vector3 pos = getPosition();
    Vector3 posP = camera->project(getPosition(), game->window_width, game->window_height);
    double meshRadius = Mesh::Load(mesh)->info.radius;
    double radius = camera->getProjectScale(pos, meshRadius*2);

    gui->addPoint(posP, color, true);
    gui->addLine(Vector3(posP.x - radius, posP.y - radius, posP.z), Vector3(posP.x + radius, posP.y - radius, posP.z), color, true);
    gui->addLine(Vector3(posP.x + radius, posP.y - radius, posP.z), Vector3(posP.x + radius, posP.y + radius, posP.z), color, true);
    gui->addLine(Vector3(posP.x + radius, posP.y + radius, posP.z), Vector3(posP.x - radius, posP.y + radius, posP.z), color, true);
    gui->addLine(Vector3(posP.x - radius, posP.y + radius, posP.z), Vector3(posP.x - radius, posP.y - radius, posP.z), color, true);

    if(stats.has_hp){
        double hpFraction = 2*radius*(double)stats.hp / (double)stats.maxhp;
        Vector3 initHP = Vector3(posP.x - radius, posP.y + 2*radius, posP.z);
        gui->addLine(initHP, initHP+Vector3(hpFraction, 0, 0), Vector4(0,1,0,1), true);
        gui->addLine(initHP+Vector3(hpFraction, 0, 0), initHP+Vector3(2*radius, 0, 0), Vector4(1,0,0,1), true);
    }

    //TODO LINEAS
    if(stats.movable){
        if(stats.followEntity){
            Entity* follow = Entity::getEntity(stats.followEntity);
            if(follow != NULL){
                if(stats.team == HUMAN_TEAM)
                    gui->addLine(pos, follow->getPosition(), Vector4(1,1,0,0.2));
                else if(stats.team == ENEMY_TEAM)
                    gui->addLine(pos, stats.targetPos, Vector4(1,0.5,0,0.2), false, true);
            }
        }else if(stats.team == HUMAN_TEAM && stats.targetPos){
            gui->addLine(pos, stats.targetPos, Vector4(1,1,1,0.2));
        }
    }

}

//================================================

EntityCollider::EntityCollider(bool dynamic) : EntityMesh(), dynamic(dynamic) {}
EntityCollider::EntityCollider():EntityCollider(false){}
EntityCollider::~EntityCollider(){
    if(this->dynamic){
        auto it = std::find(dynamic_colliders.begin(),dynamic_colliders.end(),this->uid);
        if(it != dynamic_colliders.end())
            dynamic_colliders.erase(it);
    }else{
        auto it = std::find(static_colliders.begin(),static_colliders.end(),this->uid);
        if(it != static_colliders.end())
            static_colliders.erase(it);
    }
}

//Static methods
void EntityCollider::registerCollider(EntityCollider* e) {
    if(e->dynamic){
        EntityCollider::dynamic_colliders.push_back(e->uid);
    }else{
        EntityCollider::static_colliders.push_back(e->uid);
    }
}

void EntityCollider::checkCollisions(float elapsed_time) {
    //Test every possible collision

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
            }

            //Gravitational force--------
            double dest_radius = Mesh::Load(entityDest->mesh)->info.radius;
            double total_radius = source_radius + dest_radius;
            Vector3 gravDir = entitySource->getPosition() - entityDest->getPosition();
            double distance = gravDir.length();

            gravDir.normalize();
            if(distance < total_radius) {
                entitySource->stats.gravity += gravDir * entitySource->stats.maxvel *(((total_radius == 0 ? distance : total_radius))/distance);
                //std::cout<<distance<< " "<< total_radius<<" grav: "<<entitySource->stats.gravity<<"\n";
                GUI::getGUI()->addLine(entitySource->getPosition(),entitySource->getPosition()+entitySource->stats.gravity, Vector4(1,1,1,1), false, true);
            }
        }

        for(int j = i+1 ; j < dynamic_colliders.size(); ++j){
            entityDest = (EntityCollider*)Entity::getEntity(dynamic_colliders[j]);
            if(entityDest == NULL) {dynamic_colliders.erase(dynamic_colliders.begin() + j); --j; continue;}
            if(entityDest->testSphereCollision(dinamic_pos_source, source_radius, collision)){
                entitySource->onCollision(entityDest);
                entityDest->onCollision(entitySource);
            }

            //Gravitational force--------
            double dest_radius = Mesh::Load(entityDest->mesh)->info.radius;
            double total_radius = source_radius + dest_radius;
            Vector3 gravDir = entitySource->getPosition() - entityDest->getPosition();
            double distance = gravDir.length();
            gravDir.normalize();
            if(distance < total_radius) {
                entitySource->stats.gravity += gravDir * entitySource->stats.maxvel *(((total_radius == 0 ? distance : total_radius))/distance);
                //std::cout<<distance<< " "<< total_radius<<" grav: "<<entitySource->stats.gravity<<"\n";
                GUI::getGUI()->addLine(entitySource->getPosition(),entitySource->getPosition()+entitySource->stats.gravity, Vector4(1,1,1,1), false, true);
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
    if(debugMode)
        std::cout<<this->mesh<<" collides with "<<withEntity->mesh<<std::endl;
}

void EntityCollider::onCollision(Bullet *withBullet) {
    if(debugMode)
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

EntityFighter::EntityFighter(bool dynamic):EntityCollider(dynamic) {

}

EntityFighter::~EntityFighter(){

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
    //START-------Si estamos cerca de la entidad que seguimos patruyala o disparale
    //If is following an entity set the current position of that entity
    UID saveFollow = 0;
    Vector3 pos = getPosition();
    if(stats.followEntity){
        Entity* follow = Entity::getEntity(stats.followEntity);
        if(follow != NULL){
            Vector3 followPos = follow->getPosition();
            if((followPos-pos).length() > stats.range*.6){
                stats.targetPos = follow->getPosition();
                stats.vel = stats.maxvel;
            }

            //MISMA LOGICA QUE LA GRAVEDAD PARA QUEDARSE PATRUYANDO
            float source_radius = Mesh::Load(this->mesh)->info.radius;
            Entity* entitySource = this;
            Entity* entityDest = follow;

            double dest_radius = Mesh::Load(((EntityMesh*)entityDest)->mesh)->info.radius;
            double total_radius = source_radius + dest_radius;
            Vector3 gravDir = entitySource->getPosition() - entityDest->getPosition();
            double distance = gravDir.length();

            gravDir.normalize();
            if(distance < total_radius+100) { //TODOS LOS RANGOS DE DISPARO TIENEN QUE SER MAYORES A LA MITAD DE ESTO
                saveFollow = stats.followEntity;
            }
        }
        else stats.followEntity = 0;
    }
    //END-------Si estamos cerca de la entidad que seguimos patruyala o disparale



    updateStatsAndEntityActions(elapsed_time);
    lastFireSec+=elapsed_time;
    Entity* enemy = NULL;
    if(!stats.targetPos || stats.followEntity){
        double distance = INFINITY;
        if(stats.followEntity){
            Entity* e = Entity::getEntity(stats.followEntity);
            if(e == NULL) {
                stats.followEntity = 0;
            }else
            if(e->stats.team != this->stats.team)
                enemy = e;
        }

        if(enemy == NULL)
        for(Entity* focus : Game::instance->getEnemyTeamPlayer(this->stats.team)->getControllableEntities()){
            double d = (focus->getPosition() - getPosition()).length();
            if(d < distance){
                distance = d;
                enemy = focus;
            }
        }

        if(enemy != NULL){
            if(!stats.targetPos)
                lookPosition(elapsed_time,enemy->getPosition());
            if( (enemy->getPosition() - getPosition()).length() < stats.range ){
                if(lastFireSec > 1.0/fireRate) {
                    shoot();
                    lastFireSec = 0;
                }
            }
        }else{
            stats.vel = 0;
        }
    }

    //RESTORE-------Si estamos cerca de la entidad que seguimos patruyala o disparale
    if(saveFollow)
        stats.followEntity = saveFollow;

    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}

void EntityFighter::updateGUI() {
    unitGUI();

    for(int i=0; i<children.size(); i++){
        children[i]->updateGUI();
    }
}

//==================================================

EntityStation::EntityStation(bool dynamic) : EntityCollider(dynamic) {

}

EntityStation::~EntityStation() {}

EntityStation* EntityStation::clone() {
    EntityStation* clon = new EntityStation();
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    return clon;
}

void EntityStation::update(float elapsed_time) {
    updateStatsAndEntityActions(elapsed_time);
    unitGUI();

    for(int i=0; i<children.size(); i++){
        children[i]->update(elapsed_time);
    }
}

void EntityStation::updateGUI() {
    unitGUI();

    for(int i=0; i<children.size(); i++){
        children[i]->updateGUI();
    }
}

//=====================================

float getRandomFloat(){
    float r = float(random()%1000)/100.0;
    return r;
}

EntityAsteroid::EntityAsteroid(bool dynamic):EntityCollider(dynamic) {

    rotationAxis = Vector3(getRandomFloat(),getRandomFloat(),getRandomFloat()).normalize();
    rotationVelocity = getRandomFloat()*3.0 + 3.0;
    model.rotateLocal(random()/rotationVelocity,rotationAxis);
}

EntityAsteroid::~EntityAsteroid() {

}

void EntityAsteroid::update(float elapsed_time) {
    Entity::update(elapsed_time);
    model.rotateLocal(elapsed_time/rotationVelocity,rotationAxis);
}

EntityAsteroid* EntityAsteroid::clone() {
    EntityAsteroid* clon = new EntityAsteroid(this->dynamic);
    UID uid = clon->uid;
    *clon = *this;
    clon->uid = uid;
    clon->rotationAxis = Vector3(getRandomFloat(),getRandomFloat(),getRandomFloat()).normalize();
    clon->rotationVelocity = getRandomFloat()*3.0 + 3.0;
    clon->model.rotateLocal(random()/rotationVelocity,rotationAxis);
    return clon;
}