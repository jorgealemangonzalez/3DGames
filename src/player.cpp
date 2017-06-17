
#include "player.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"

Player::Player(){

}

Player::~Player() {}

void Player::addControllableEntity(UID e_uid) {
    controllable_entities.push_back(e_uid);
}

std::vector<Entity*> Player::getControllableEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllable_entities.begin(); it != controllable_entities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllable_entities.erase(it);

    return entities;
}

//========================================

Human::Human() {
    cameraController = new CameraController();
    entityController = new FighterController();
}

Human::~Human() {

}

void Human::update(double seconds_elapsed) {
cameraController->update(seconds_elapsed,12);//TODO Quit entity from camera controller
}

void Human::render(Camera *camera) {

}

void Human::centerCameraOnControlling(){
    Vector3 dir = Game::instance->camera->eye - center_controlling;
    dir.normalize();
    Camera* camera = Game::instance->camera;
    camera->eye = dir * (radius_controlling+100) + center_controlling;
    camera->center = center_controlling;
}

void Human::selectEntities(std::vector<UID>& entities) {
    controlling_entities = entities;
    std::vector<Entity *> control_entities = getControllingEntities();
    if(control_entities.size()) {
        center_controlling = Vector3();

        for (Entity *e: control_entities)
            center_controlling += e->getPosition();

        center_controlling /= control_entities.size();

        radius_controlling = 0;
        Entity *fartest;
        for (Entity *e: control_entities) {
            float dist = e->getPosition().distance(center_controlling);
            if (dist >= radius_controlling) {
                radius_controlling = dist;
                fartest = e;
            }
        }
        radius_controlling += Mesh::Load(
                ((EntityMesh *) fartest)->mesh)->info.radius;        //TODO Solo se pueden seleccionar entity mesh
        //Put the grid in the center
        //TODO update it when selected moves

        GUI::getGUI()->setGridCenter(center_controlling);
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
            controlling[i]->stats.vel = 100;
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
                               Vector3( (jump+1)*50 + radius, 0, 0);
            else
                move = position -
                               Vector3( (jump+1)*50 + radius, 0, 0);
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.vel = 100;
        }
    }
}

void Human::moveSelectedInPlane(){
    Vector3 move_position; //LO QUE QUEREMOS CALCULAR
    Game* g = Game::instance;
    Camera* camera = g->camera;
    GUI *gui = GUI::getGUI();
    //TODO cuidado , si se mueve la camara mientras seleccionas donde mover el objetivo entonces esto no funciona ¿ O si ?

    Vector3 initCameraToGoal= camera->eye;
    Vector3 cameraToGoal= camera->unproject(Vector3(g->mouse_when_press.x,g->window_height - g->mouse_position.y,0)
                                         ,g->window_width, g->window_height)
                                            - camera->eye;
    Vector3 initStartInPlaneToGoal;//Interseccion del rayo con la grid
    Vector3 auxDir = camera->unproject(Vector3(g->mouse_when_press.x,g->mouse_when_press.y,0),
                                                        g->window_width,g->window_height) - camera->eye;
    gui->grid->testRayCollision(camera->eye,auxDir,1000000000.0,initStartInPlaneToGoal);
    Vector3 startInPlaneToGoal = camera->unproject(Vector3(g->mouse_when_press.x,g->window_height - g->mouse_position.y,0),g->window_width,g->window_height)
    -camera->unproject(Vector3(g->mouse_when_press.x,g->mouse_when_press.y,0),g->window_width,g->window_height);

    if(startInPlaneToGoal == Vector3(0,0,0))
        move_position = initStartInPlaneToGoal;
    else{
        //ALGEBRA PARA SACAR EL PUNTO ELEVADO E = eye , G = Goal= move_position , P = punto en plano
        //Mediante la intersección de rectas en forma parametrica 3D ( a manija )
        Vector3 E = initCameraToGoal, P = initStartInPlaneToGoal;
        //Vectores unitarios
        Vector3 EG = cameraToGoal.normalize(), PG = startInPlaneToGoal.normalize();

        float ecuacion1 = -E.y + P.y - (EG.y*P.x)/EG.x + (E.x*EG.y)/EG.x;
        float ecuacion2 = (PG.x*EG.y)/EG.x - PG.y;
        float modulo = ecuacion1 / ecuacion2;
        modulo = (modulo < 0.0 ? -modulo : modulo); //VALOR ABSOLUTO

        move_position = PG*modulo + P;
    }

    organizeSquadCircle(move_position);
}

std::vector<Entity*> Human::getControllingEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controlling_entities.begin(); it != controlling_entities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controlling_entities.erase(it);

    return entities;
}

const float &Human::getRadius_controlling() const {
    return radius_controlling;
}

const Vector3 &Human::getCenter_controlling() const {
    return center_controlling;
}

//========================================

Enemy::Enemy() {
    aiController = new AIController();
}

Enemy::~Enemy() {

}

void Enemy::update(double seconds_elapsed) {
    for(Entity* e: getControllableEntities()){
        aiController->update(seconds_elapsed,e);
    }
}