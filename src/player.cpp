//
// Created by jorgealemangonzalez on 5/05/17.
//

#include "player.h"
Player::Player() {
    cameraController = new CameraController();
}

Player::~Player() {

}

void Player::update(double seconds_elapsed) {
    cameraController->update(seconds_elapsed);
}

void Player::setMyEntity(Entity *entity) {

}
