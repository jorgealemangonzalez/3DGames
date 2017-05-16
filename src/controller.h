
#ifndef TJE_FRAMEWORK_2017_CONTROLLER_H
#define TJE_FRAMEWORK_2017_CONTROLLER_H

#include "constants.h"
#include "includes.h"
#include "entity.h"

class CameraController{
public:
    int mode;
    Vector3 entityPreviusPos;
    //Methods
    CameraController();
    ~CameraController();

    void setMode(int m);
    void notifyEntity(UID e_uid);
    void update( double seconds_elapsed, UID e_uid);
};

class EntityController{
public:
    EntityController();
    ~EntityController();

    virtual void update( double seconds_elapsed, UID e_uid);
};

class FighterController : public EntityController{
public:
    float acc; //Linear acceleration
    float vel; //Linear velocity

    float angX; //Angular acceleration in axis X
    float angY; //Angular acceleration in axis Y

    FighterController();
    ~FighterController();

    void update( double seconds_elapsed, UID e_uid);
};

class AIController : public EntityController{
public:
    UID entity_follow;

    AIController();
    ~AIController();

    void update( double seconds_elapsed, UID e_uid);
};

class ClickController : public EntityController{
public:
    ClickController();
    ~ClickController();

    void update( double seconds_elapsed, UID e_uid);
};

#endif //TJE_FRAMEWORK_2017_CONTROLLER_H

