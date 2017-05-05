
#ifndef TJE_FRAMEWORK_2017_CONTROLLER_H
#define TJE_FRAMEWORK_2017_CONTROLLER_H

#include "includes.h"
#include "entity.h"

class CameraController{
public:
    int mode;
    Entity* entity;

    //Methods
    CameraController();
    ~CameraController();

    void setEntity(Entity* e);
    void setMode(int m);

    void update( double seconds_elapsed );
};

class EntityController{
public:
    Entity* entity;

    EntityController();
    ~EntityController();

    void setEntity(Entity* e);
    virtual void update( double seconds_elapsed );
};

class FighterController: public EntityController{
public:
    float acc; //Linear acceleration
    float vel; //Linear velocity

    float angX; //Angular acceleration in axis X
    float angY; //Angular acceleration in axis Y

    FighterController();
    ~FighterController();

    void update( double seconds_elapsed );
};

#endif //TJE_FRAMEWORK_2017_CONTROLLER_H

