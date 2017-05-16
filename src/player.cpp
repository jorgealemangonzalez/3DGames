
#include "player.h"
#include "game.h"

Player::Player(){

}

Player::~Player() {}

void Player::addControllableEntity(UID e_uid) {
    controllable_entities.push_back(e_uid);
}

//========================================

void Human::rotateControlling() {
    controlling_entity = (controlling_entity+1)%controllable_entities.size();
    cameraController->notifyEntity(controllable_entities[controlling_entity]);
}

Human::Human() {
    cameraController = new CameraController();
    entityController = new FighterController();
}

Human::~Human() {

}

void Human::update(double seconds_elapsed) {

    entityController->update(seconds_elapsed, controllable_entities[controlling_entity]);
    cameraController->update(seconds_elapsed, controllable_entities[controlling_entity]);
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