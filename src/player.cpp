
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

//========================================

Human::Human() {
    show_control_plane = false;
    cameraController = new CameraController();
    entityController = new FighterController();
    std::string texture = "grid.tga";
    Texture::Load(texture,true);
    Mesh *plane = new Mesh();
    plane->createQuad(0,0,1000,1000);
    float tam = 100.f;
    plane->uvs[0] = Vector2(tam,tam);
    plane->uvs[1] = Vector2(0.0f,0.0f);
    plane->uvs[2] = Vector2(tam,0.0f);
    plane->uvs[3] = Vector2(0.0f,tam);
    plane->uvs[4] = Vector2(0.0f,0.0f);
    plane->uvs[5] = Vector2(tam,tam);
    plane->info.radius = tam*2;
    Mesh::s_Meshes["_grid"] = plane;

    grid = new EntityCollider();
    grid->setMesh("_grid");
    grid->setTexture(texture);
}

Human::~Human() {

}

void Human::update(double seconds_elapsed) {
cameraController->update(seconds_elapsed,12);//TODO Quit entity from camera controller
}

void Human::showHideControlPlane(){
    show_control_plane = !show_control_plane;
}

void Human::render(Camera *camera) {
    if(show_control_plane){
        grid->model.setTranslation(center_controlling);
        Vector3 axis(1.0,0,0);
        grid->model.rotateLocal(DEG2RAD*90,axis);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid->render(camera);
        glDisable(GL_BLEND);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
}

void Human::centerCameraOnControlling(){
    Vector3 dir = Game::instance->camera->eye - center_controlling;
    dir.normalize();
    Camera* camera = Game::instance->camera;
    camera->eye = dir * 10 *radius_controlling + center_controlling;
    camera->center = center_controlling;
}

void Human::selectEntities(std::vector<UID>& entities) {
    controlling_entities = entities;
    if(controlling_entities.size()) {
        center_controlling = Vector3();
        std::vector<Entity *> control_entities;
        for (UID e: controlling_entities)
            control_entities.push_back(Entity::getEntity(e));

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
    }
}

void Human::moveSelectedInPlane(Vector3 positionRay, Vector3 directionRay){
    if(controlling_entities.size()){
        Vector3 move_position;
        if(grid->testRayCollision(positionRay,directionRay,10000000.0,move_position)) {
            for(UID controlling_entity : controlling_entities) {
                Entity *control = Entity::getEntity(controlling_entity);
                control->stats.targetPos = move_position;
                control->stats.vel = 100;
                std::cout << "MOVE_POSITION: " << move_position << "\n";
            }
        }
    }
}

const std::vector<UID> &Human::getControlling_entities() const {
    return controlling_entities;
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
    for(int i = 0 ; i < controllable_entities.size(); ++i){
        aiController->update(seconds_elapsed,controllable_entities[i]);
    }
}