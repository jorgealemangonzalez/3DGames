
#include "player.h"
Player::Player() : controlling_entity(0) {
    controllable_entities.push_back(0);

    cameraController = new CameraController();
    entityController = new FighterController();
}

Player::~Player() {}

void Player::update(double seconds_elapsed) {
    entityController->update(seconds_elapsed, controllable_entities[controlling_entity]);
    cameraController->update(seconds_elapsed, controllable_entities[controlling_entity]);
}

void Player::addControllableEntity(UID e_uid) {
    controllable_entities.push_back(e_uid);
}

void Player::rotateControlling() {
    controlling_entity = (controlling_entity+1)%controllable_entities.size();
    cameraController->notifyEntity(controllable_entities[controlling_entity]);
}