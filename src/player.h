
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
    std::vector<UID> controlling_entities;            // Index of controllable pool
    float radius_controlling;
    Vector3 center_controlling;
    bool show_control_plane;
public:
    Human();
    ~Human();

    EntityController* entityController;
    CameraController* cameraController;
    EntityCollider* grid;

    const std::vector<UID> &getControlling_entities() const;

    const float &getRadius_controlling() const;

    const Vector3 &getCenter_controlling() const;

    void selectEntities(std::vector<UID>& entities);
    void centerCameraOnControlling();
    void moveSelectedInPlane(Vector3 positionRay, Vector3 directionRay);
    void render(Camera* camera);
    void update(double seconds_elapsed);
    void showHideControlPlane();

};

class Enemy: public Player{
public:
    Enemy();
    ~Enemy();

    AIController* aiController;

    void update(double seconds_elapsed);
};

#endif //TJE_FRAMEWORK_2017_PLAYER_H
