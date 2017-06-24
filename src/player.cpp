
#include "player.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"

Player::Player(std::string t) : team(t){

}

Player::~Player() {}

void Player::addControllableEntity(UID e_uid) {
    controllableEntities.push_back(e_uid);
}

std::vector<Entity*> Player::getControllableEntities(){
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

//========================================

Human::Human() : Player(HUMAN_TEAM) {
    cameraController = new CameraController();
    entityController = new FighterController();
    updateCenter = true;
}

Human::~Human() {

}

void Human::update(double seconds_elapsed) {
    cameraController->update(seconds_elapsed,12); //TODO Quit entity from camera controller

    std::vector<Entity*> controlling = getControllingEntities();
    Vector3 center;
    for(Entity* e : controlling){
        center += e->getPosition();
    }

    if(updateCenter){
        center /= controlling.size();
        centerControlling = center;
        if(controlling.size())
            Game::instance->camera->center = centerControlling;
        GUI::getGUI()->setGrid((bool)controllingEntities.size(), centerControlling);
    }
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

//Si el click inicial no colisiona con la grid no iba bien, esta función lo soluciona.
bool Human::getPositionSelectedMove(Vector3 &selectedMove) {

    Vector3 move_position; //LO QUE QUEREMOS CALCULAR
    Game* g = Game::instance;
    Camera* camera = g->camera;
    GUI *gui = GUI::getGUI();

    Vector3 initCameraToGoal= camera->eye;
    Vector3 cameraToGoal= camera->unproject(Vector3(g->mouse_when_press.x, g->mouse_position.y,0)
            ,g->window_width, g->window_height)
                          - camera->eye;
    Vector3 initStartInPlaneToGoal;//Interseccion del rayo con la grid
    Vector3 auxDir = camera->unproject(Vector3(g->mouse_when_press.x,g->mouse_when_press.y,0),
                                       g->window_width,g->window_height) - camera->eye;
    if(!gui->grid->testRayCollision(camera->eye,auxDir,1000000000.0,initStartInPlaneToGoal))
        return false;
    Vector3 startInPlaneToGoal = camera->unproject(Vector3(g->mouse_when_press.x, g->mouse_position.y,0),g->window_width,g->window_height)
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
    selectedMove = move_position;
    return true;

    /*  Va bien, pero mismo comportamiento raro de no sacar siempre el punto vertical...
     *  Eso si, parece más limpio y directo. Si quieres poner este, tu mismo.
     *  Solución con teorema de Tales
    Game* g = Game::instance;
    Camera* camera = g->camera;
    GUI *gui = GUI::getGUI();

    Vector3 O = camera->eye;
    Vector3 A = camera->unproject(Vector2(g->mouse_when_press.x, g->mouse_position.y), g->window_width, g->window_height);
    Vector3 B = camera->unproject(Vector2(g->mouse_when_press), g->window_width, g->window_height);

    Vector3 Bp;
    Vector3 dir = B-O;
    if(!gui->grid->testRayCollision(camera->eye,dir,INFINITY,Bp))
        return false;

    double dist = ( (O-A).length() * (O-Bp).length() ) / (O-B).length();
    Vector3 Ap = O + (A-O).normalize()*dist;
    selectedMove = Ap;
    return true;
     */
}

void Human::moveSelectedInPlane(){
    Vector3 selectedMove;
    if(getPositionSelectedMove(selectedMove))
        organizeSquadCircle(selectedMove);
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
    aiController = new AIController();
}

Enemy::~Enemy() {

}

void Enemy::update(double seconds_elapsed) {
    for(Entity* e: getControllableEntities()){
        aiController->update(seconds_elapsed,e);
    }
}