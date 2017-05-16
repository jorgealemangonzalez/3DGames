
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

    std::vector<UID> controllable_entities;     // Pool of controllable (friendly) entities


    void addControllableEntity(UID e_uid);
    virtual void update(double seconds_elapsed)= 0;
};

class Human: public Player{
public:
    Human();
    ~Human();

    EntityController* entityController;
    CameraController* cameraController;

    UID selected_entity;                        // For selecting with mouse
    unsigned int controlling_entity;            // Index of controllable pool
    void rotateControlling();
    void setSelectedEntity(UID e_uid);
    void update(double seconds_elapsed);

};

class Enemy: public Player{
public:
    Enemy();
    ~Enemy();

    AIController* aiController;

    void update(double seconds_elapsed);
};

#endif //TJE_FRAMEWORK_2017_PLAYER_H
