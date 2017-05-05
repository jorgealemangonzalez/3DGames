
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

#endif //TJE_FRAMEWORK_2017_CONTROLLER_H

