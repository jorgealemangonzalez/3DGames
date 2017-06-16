
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include <vector>
#include "constants.h"
#include "entity.h"
#include "controller.h"

class Player {
    std::vector<UID> controllable_entities;     // Pool of controllable (friendly) entities
public:
    Player();
    ~Player();


    std::vector<Entity*> getControllableEntities();

    void addControllableEntity(UID e_uid);
    virtual void update(double seconds_elapsed)= 0;
};

class Human: public Player{
    std::vector<UID> controlling_entities;            // Index of controllable pool
    float radius_controlling;
    Vector3 center_controlling;
public:
    Human();
    ~Human();

    EntityController* entityController;
    CameraController* cameraController;

    std::vector<Entity*> getControllingEntities(); //Controla si una de las unidades ha desaparecido

    const float &getRadius_controlling() const;

    const Vector3 &getCenter_controlling() const;

    void selectEntities(std::vector<UID>& entities);
    void centerCameraOnControlling();
    void moveSelectedInPlane();
    void render(Camera* camera);
    void update(double seconds_elapsed);

    void organizeSquadCircle(Vector3 position);
    void organizeSquadLine(Vector3 position);

};

class Enemy: public Player{
public:
    Enemy();
    ~Enemy();

    AIController* aiController;

    void update(double seconds_elapsed);
};

#endif //TJE_FRAMEWORK_2017_PLAYER_H
