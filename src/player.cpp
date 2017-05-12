
#include "player.h"
Player::Player() {
    cameraController = new CameraController();
    entityController = new FighterController();
}

Player::~Player() {

}

void Player::update(double seconds_elapsed) {
    entityController->update(seconds_elapsed);
    cameraController->update(seconds_elapsed);
}

void Player::setMyEntity(unsigned int e_uid) {
    entityController->setEntity(e_uid);
    cameraController->setEntity(e_uid);
}
