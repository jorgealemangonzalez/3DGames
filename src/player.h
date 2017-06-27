
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include <vector>
#include "constants.h"
#include "entity.h"
#include "controller.h"

class Player {
protected:
    std::vector<UID> controllableEntities;      // Pool of controllable (friendly) entities
    std::string team;

public:
    Player(std::string t);
    virtual ~Player();

    std::vector<UID> maintainAliveEntities;     // List of entities that keep the player without losing

    std::vector<Entity*> getControllableEntities();

    void addControllableEntity(UID e_uid);
    virtual void update(double seconds_elapsed)= 0;
};

class Human: public Player{
protected:
    std::vector<UID> controllingEntities;            // Index of controllable pool
    float radiusControlling;
    Vector3 centerControlling;
public:
    Human();
    virtual ~Human();

    CameraController* cameraController;
    bool updateCenter;

    std::vector<Entity*> getControllingEntities(); //Controla si una de las unidades ha desaparecido
    std::vector<Entity*> getControllableEntities();

    const float &getRadiusControlling() const;
    const Vector3 &getCenterControlling() const;

    void selectAllEntities();
    void followEntitie(UID follow);
    void selectEntities(std::vector<UID>& entities);
    void centerCameraOnControlling();
    void moveSelectedInPlane();
    void render(Camera* camera);
    void update(double seconds_elapsed);

    void organizeSquadCircle(Vector3 position);
    void organizeSquadLine(Vector3 position);
    Vector3 getPositionSelectedMove();
    bool getPositionSelectedMove(Vector3 &selectedMove);

};

class Enemy: public Player{
public:
    Enemy();
    virtual ~Enemy();

    void update(double seconds_elapsed);
};

#endif //TJE_FRAMEWORK_2017_PLAYER_H
