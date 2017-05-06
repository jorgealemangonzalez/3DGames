
#ifndef TJE_FRAMEWORK_2017_CONTROLLER_H
#define TJE_FRAMEWORK_2017_CONTROLLER_H

#include "includes.h"
#include "entity.h"

class EntityController{
public:
    Entity* entity;

    EntityController();
    ~EntityController();

    virtual void setEntity(Entity* e);
    virtual void update( double seconds_elapsed );
};

class CameraController: public EntityController{
public:
    int mode;
    Vector3 entityPreviusPos;
    //Methods
    CameraController();
    ~CameraController();


    void setMode(int m);
    void setEntity(Entity* e);

    void update( double seconds_elapsed );
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

