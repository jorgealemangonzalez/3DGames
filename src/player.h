
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include <vector>
#include "constants.h"
#include "entity.h"
#include "controller.h"

class Player {

public:
    Player();
    ~Player();

    unsigned int controlling_entity;            // Index of controllable pool
    std::vector<UID> controllable_entities;     // Pool of controllable (friendly) entities
    UID selected_entity;                        // For selecting with mouse

    EntityController* entityController;
    CameraController* cameraController;

    void addControllableEntity(UID e_uid);
    void setSelectedEntity(UID e_uid);
    void rotateControlling();
    void update(double seconds_elapsed);
};


#endif //TJE_FRAMEWORK_2017_PLAYER_H
