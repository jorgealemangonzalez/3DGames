
#include "player.h"
Player::Player() {
    cameraController = new CameraController();
    entityController = new EntityController();
}

Player::~Player() {

}

void Player::update(double seconds_elapsed) {
    entityController->update(seconds_elapsed);
    cameraController->update(seconds_elapsed);
}

void Player::setMyEntity(Entity *entity) {
    entityController->setEntity(entity);
    cameraController->setEntity(entity);
}
