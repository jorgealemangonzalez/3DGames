
#include "player.h"
Player::Player() : entity_uid(0) {
    cameraController = new CameraController();
    entityController = new FighterController();
}

Player::~Player() {}

void Player::update(double seconds_elapsed) {
    entityController->update(seconds_elapsed, entity_uid);
    cameraController->update(seconds_elapsed, entity_uid);
}

void Player::setMyEntity(unsigned int e_uid) {
    entity_uid = e_uid;
    cameraController->notifyEntity(e_uid);
}
