
#ifndef TJE_FRAMEWORK_2017_PLAYER_H
#define TJE_FRAMEWORK_2017_PLAYER_H

#include <vector>
#include <set>
#include "constants.h"
#include "entity.h"
#include "controller.h"
#define DEFEND_SIZE 5

class Player {
protected:



public:
    std::vector<UID> controllableEntities;  // Pool of controllable (friendly) entities
    std::string team;
    Player(std::string t);
    virtual ~Player();

    std::vector<UID> maintainAliveEntities;     // List of entities that keep the player without losing

    std::vector<Entity*> getControllableEntities();

    virtual void addControllableEntity(UID e_uid) = 0;
    virtual void update(double seconds_elapsed)= 0;
};

class Human: public Player{
protected:

    float radiusControlling;
    Vector3 centerControlling;
public:
    Human();
    virtual ~Human();

    CameraController* cameraController;
    bool updateCenter;
    bool organizeCircle;
    std::vector<UID> controllingEntities;            // Index of controllable pool

    std::vector<Entity*> getControllingEntities(); //Controla si una de las unidades ha desaparecido
    std::vector<Entity*> getControllableEntities();

    const float &getRadiusControlling() const;
    const Vector3 &getCenterControlling() const;

    void addControllableEntity(UID e_uid);
    void selectAllEntities();
    void followEntitie(std::vector<UID>& entities);
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
    std::vector<UID> defend_squad;      //Defiende la base
    std::vector<UID> attack_squad;      //Ataca a las unidades del humano
    std::vector<UID> aniquile_squad;    //Ataca a las unidades mantainAlive del humano
    std::vector<UID> to_defend;
    std::vector<UID> to_attack_human;
    std::vector<UID> to_aniquile_human;

    Enemy();
    virtual ~Enemy();
    void addControllableEntity(UID e_uid);
    void equilibrateSquads();
    void update(double seconds_elapsed);
};

#endif //TJE_FRAMEWORK_2017_PLAYER_H
