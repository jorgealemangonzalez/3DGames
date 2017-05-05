//
// Created by jorgealemangonzalez on 5/05/17.
//

#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include "entity.h"
#include "controller.h"
class Player {

public:
    Player();
    ~Player();

    CameraController* cameraController;

    void setMyEntity(Entity* entity);
    void update(double seconds_elapsed);
};


#endif //TJE_FRAMEWORK_2017_PLAYER_H
