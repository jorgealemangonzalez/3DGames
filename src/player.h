
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include "entity.h"
#include "controller.h"
class Player {

public:
    Player();
    ~Player();

    EntityController* entityController;
    CameraController* cameraController;

    void setMyEntity(Entity* entity);
    void update(double seconds_elapsed);
};


#endif //TJE_FRAMEWORK_2017_PLAYER_H
