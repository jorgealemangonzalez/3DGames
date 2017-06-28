
#include "player.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"

Player::Player(std::string t) : team(t){

}

Player::~Player() {
    for(Entity* e : getControllableEntities()){
        delete e;
    }
}

std::vector<Entity*> Player::getControllableEntities(){
    return Entity::getAndCleanEntityVector(controllableEntities);
}

//========================================

Human::Human() : Player(HUMAN_TEAM) {
    cameraController = new CameraController();
    updateCenter = true;
    organizeCircle = true;
}

Human::~Human() {
    delete cameraController;
}

void Human::addControllableEntity(UID e_uid) {
    controllableEntities.push_back(e_uid);

    Entity* e = Entity::getEntity(e_uid);
    if(e->stats.mantainAlive)
        Game::instance->enemy->to_aniquile_human.push_back(e_uid);
    else
        Game::instance->enemy->to_attack_human.push_back(e_uid);

}

void Human::update(double seconds_elapsed) {
    cameraController->update(seconds_elapsed); //TODO Quit entity from camera controller

    std::vector<Entity*> controlling = getControllingEntities();
    Vector3 center;
    for(Entity* e : controlling){
        center += e->getPosition();
    }

    if(updateCenter && controlling.size()){
        center /= controlling.size();
        centerControlling = center;
        if(controlling.size()) {
            Camera* camera = Game::instance->camera;
            Vector3 dif = centerControlling - camera->center;
            camera->eye = camera->eye + dif;
            camera->center = centerControlling;
        }
    }
    GUI::getGUI()->setGrid((bool)controlling.size(), centerControlling);
}

void Human::render(Camera *camera) {

}

void Human::centerCameraOnControlling(){
    if(!controllingEntities.size()) return;
    Vector3 dir = Game::instance->camera->eye - centerControlling;
    dir.normalize();
    Camera* camera = Game::instance->camera;
    camera->eye = dir * (radiusControlling+100) + centerControlling;
    camera->center = centerControlling;
}
void Human::selectAllEntities(){
    std::vector<UID> to_select;
    for(UID eu: this->controllableEntities){
        Entity* e = Entity::getEntity(eu);
        if(e == NULL)
            continue;
        if(e->stats.movable)
            to_select.push_back(e->uid);
    }
    std::cout<<to_select.size();
    this->selectEntities(to_select);
}

void Human::followEntitie(std::vector<UID>& entities) {
    //It prefers to follow enemy entities rather than others (miquel: haré algún cambio más al tema de seleccionar objetivos para human)
    UID nearest = 0;
    float minDist = INFINITY;
    Vector3 cameraPos = Game::instance->camera->eye;
    Vector3 cameraFront = Game::instance->camera->center - cameraPos;
    for(Entity* e : Entity::getAndCleanEntityVector(entities)){
        float dist = cameraPos.distance(e->getPosition());
        if(dist < minDist && cameraFront.dot(e->getPosition() - cameraPos) > 0.0){
            nearest = e->uid;
            minDist = dist;
        }
    }
    for(Entity* control : getControllingEntities()){
        control->stats.followEntity = nearest;
    }
}

void Human::selectEntities(std::vector<UID>& entities) {
    controllingEntities = entities;
    std::vector<Entity *> controlEntities = getControllingEntities();

    if(controlEntities.size()) {
        Vector3 center;
        for (Entity *e: controlEntities) {
            center += e->getPosition();
        }
        center /= controlEntities.size();
        centerControlling = center;

        Entity *farthest = NULL;
        radiusControlling = 0;
        for (Entity *e: controlEntities) {
            float dist = e->getPosition().distance(centerControlling);
            if (dist >= radiusControlling) {
                radiusControlling = dist;
                farthest = e;
            }
        }

        if(farthest != NULL)
            radiusControlling += Mesh::Load(((EntityMesh *) farthest)->mesh)->info.radius;
    }
}

void Human::organizeSquadCircle(Vector3 position){
    //Intentemos crear un circulo y poner las naves en este
    std::vector<Entity*> controlling = getControllingEntities();
    if(controlling.size()) {
        float acum_sizes = 0.0;
        for (Entity *e : controlling) {
            acum_sizes += Mesh::Load(((EntityMesh *) e)->mesh)->info.radius * 2 + 50;
        }

        //L = 2 * PI * r
        float circ_r = acum_sizes / (2.0 * PI);
        float section = (360.0 / (float) controlling.size());
        for (unsigned int i = 0; i < controlling.size(); ++i) {
            Vector3 move = position +
                           Vector3(circ_r * cos((i * section) * DEG2RAD), 0, circ_r * sin((i * section) * DEG2RAD));
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.vel = controlling[i]->stats.maxvel;
        }
    }
}

void Human::organizeSquadLine(Vector3 position){
    //Intentemos crear un circulo y poner las naves en este
    std::vector<Entity*> controlling = getControllingEntities();
    if(controlling.size()) {
        for (unsigned int i = 0; i < controlling.size(); ++i) {
            int jump = i/2;
            float radius = Mesh::Load(((EntityMesh*)controlling[i])->mesh)->info.radius;
            Vector3 move;
            if(i%2)
                move = position +
                               Vector3(0, 0, (jump+1)*50 + radius);
            else
                move = position -
                               Vector3(0, 0, (jump+1)*50 + radius);
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.vel = controlling[i]->stats.maxvel;
        }
    }
}

bool Human::getPositionSelectedMove(Vector3 &selectedMove) {
    Game* g = Game::instance;
    Camera* camera = g->camera;
    GUI *gui = GUI::getGUI();

    Vector3 O = camera->eye;
    Vector3 A = camera->unproject(Vector3(g->mouse_when_press.x, g->mouse_position.y, 0.5), g->window_width, g->window_height);
    Vector3 B = camera->unproject(Vector3(g->mouse_when_press, 0.5), g->window_width, g->window_height);

    Vector3 Bp;
    Vector3 dir = (B-O).normalize();
    if(!gui->grid->testRayCollision(camera->eye,dir,INFINITY,Bp))
        return false;

    /*double dist = ( (O-A).length() * (O-Bp).length() ) / (O-B).length();
    Vector3 Ap = B + (A-O).normalize()*dist;
    selectedMove = Ap;
    return true;*/

    Mesh* mesh_grid = Mesh::Load(gui->grid->mesh);
    Vector3 N = gui->grid->getGlobalModel().rotateVector(mesh_grid->normals[0]).normalize();

    double dist = ( (B-A).length() * (O-Bp).length() ) / (O-B).length();
    Vector3 Ap = Bp + N*dist*(g->mouse_when_press.y < g->mouse_position.y ? 1 : -1);
    selectedMove = Ap;
    return true;
}



void Human::moveSelectedInPlane(){
    for(Entity* en : getControllingEntities()){
        en->stats.followEntity = 0;
    }

    Vector3 selectedMove;
    if(getPositionSelectedMove(selectedMove)){
        if(organizeCircle)
            organizeSquadCircle(selectedMove);
        else
            organizeSquadLine(selectedMove);
    }
}

std::vector<Entity*> Human::getControllingEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllingEntities.begin(); it != controllingEntities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllingEntities.erase(it);

    return entities;
}

std::vector<Entity*> Human::getControllableEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllableEntities.begin(); it != controllableEntities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllableEntities.erase(it);

    return entities;
}

const float &Human::getRadiusControlling() const {
    return radiusControlling;
}

const Vector3 &Human::getCenterControlling() const {
    return centerControlling;
}

//========================================

Enemy::Enemy() : Player(ENEMY_TEAM) {
}

Enemy::~Enemy() {

}

void Enemy::addControllableEntity(UID e_uid) {
    if(Entity::getEntity(e_uid)->stats.mantainAlive)
        to_defend.push_back(e_uid);
    controllableEntities.push_back(e_uid);
    if(defend_squad.size() < DEFEND_SIZE)
        defend_squad.push_back(e_uid);
    else{

        if(attack_squad.size() == aniquile_squad.size())
            attack_squad.push_back(e_uid);
        else if(attack_squad.size() < aniquile_squad.size())
            attack_squad.push_back(e_uid);
        else
            aniquile_squad.push_back(e_uid);
    }
}

void Enemy::equilibrateSquads() {
    if(defend_squad.size() < DEFEND_SIZE){                           //Prioriza la defensa
        if(attack_squad.size()) {                           //2º prioriza aniquilacion (Le quita al ataque de unidades)
            defend_squad.push_back(attack_squad[0]);
            attack_squad.erase(attack_squad.begin());
        }else if(aniquile_squad.size()){
            defend_squad.push_back(aniquile_squad[0]);
            aniquile_squad.erase(aniquile_squad.begin());
        }
    }else if(aniquile_squad.size() < attack_squad.size()){    //2º prioriza aniquilacion
        aniquile_squad.push_back(attack_squad[0]);
        attack_squad.erase(attack_squad.begin());
    }else if(aniquile_squad.size() > attack_squad.size()+1){
        attack_squad.push_back(aniquile_squad[0]);
        aniquile_squad.erase(aniquile_squad.begin());
    }
}

void Enemy::update(double seconds_elapsed) {
    equilibrateSquads();

    int i = 0;//TODO HACER QUE GIRE EL VECTOR CONTRARIO

    if(defend_squad.size() && to_defend.size()) {//TO DEFEND SHOULD ALLWAYS HAVE SOMETHING
        std::vector<Entity*> defend = Entity::getAndCleanEntityVector(defend_squad);
        for (Entity* defender : defend) {
            defender->stats.followEntity = to_defend[i];
            i = (i+1)%to_defend.size();
        }
    }

    i=0;
    if(attack_squad.size() && to_attack_human.size()) {
        std::vector<Entity*> attack = Entity::getAndCleanEntityVector(attack_squad);
            for (Entity* attacker : attack) {
                attacker->stats.followEntity = to_attack_human[i];
                i = (i+1)%to_attack_human.size();
            }
    }

    i = 0;
    if(aniquile_squad.size() && to_aniquile_human.size()) {
        std::vector<Entity*> aniquile =Entity::getAndCleanEntityVector(aniquile_squad);
        for (Entity* aniquiler : aniquile) {
            aniquiler->stats.followEntity = to_aniquile_human[i];
            i = (i+1)%to_aniquile_human.size();
        }
    }
}