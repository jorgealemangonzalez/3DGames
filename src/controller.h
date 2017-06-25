
#ifndef TJE_FRAMEWORK_2017_CONTROLLER_H
#define TJE_FRAMEWORK_2017_CONTROLLER_H

#include "constants.h"
#include "includes.h"
#include "entity.h"

class CameraController{
    /*TODO Meter todos los comportamientos de la camara aqui:
        - Que tenga un punto al que siempre ve a una distancia en concreto,
            ir cambiando la posición de este punto conforme lo que seleccionamos con el raton
            Hay que guardar dos vectores para reconstruir la camara con la misma inclinación
            http://tamats.com/games/ld27/
    */
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
#endif //TJE_FRAMEWORK_2017_CONTROLLER_H

