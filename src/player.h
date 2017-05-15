
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include "constants.h"
#include "entity.h"
#include "controller.h"

class Player {

public:
    Player();
    ~Player();

    UID entity_uid;

    EntityController* entityController;
    CameraController* cameraController;

    void setMyEntity(unsigned int e_uid);
    void update(double seconds_elapsed);
};


#endif //TJE_FRAMEWORK_2017_PLAYER_H
