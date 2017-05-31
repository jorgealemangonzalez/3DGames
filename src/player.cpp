
#include "player.h"
#include "game.h"

Player::Player(){

}

Player::~Player() {}

void Player::addControllableEntity(UID e_uid) {
    controllable_entities.push_back(e_uid);
}

//========================================

Human::Human() {
    cameraController = new CameraController();
    entityController = new FighterController();

    Mesh *plane = new Mesh();
    plane->createQuad(0,0,1000,1000);
    plane->uvs[0] = Vector2(10,10);
    plane->uvs[1] = Vector2(10,-10);
    plane->uvs[2] = Vector2(-10,-10);
    plane->uvs[3] = Vector2(-10,10);
    plane->uvs[4] = Vector2(10,10);
    plane->uvs[5] = Vector2(-10,-10);
    Mesh::s_Meshes["_grid"] = plane;

    grid = new EntityMesh();
    grid->setMesh("_grid");
    grid->setTexture("grid.tga");
}

Human::~Human() {

}

void Human::update(double seconds_elapsed) {

    entityController->update(seconds_elapsed, controllable_entities[controlling_entity]);
    cameraController->update(seconds_elapsed, controllable_entities[controlling_entity]);

    if(controlling_entity){
        Entity* controlling = Entity::getEntity(controlling_entity);
        grid->model.setTranslation(controlling->getPosition());
    }
}

void Human::rotateControlling() {
    controlling_entity = (controlling_entity+1)%controllable_entities.size();
    cameraController->notifyEntity(controllable_entities[controlling_entity]);
}

void Human::render(Camera *camera) {
    if(controlling_entity){



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

void Human::selectEntity(UID e_uid) {
    controlling_entity = e_uid;
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