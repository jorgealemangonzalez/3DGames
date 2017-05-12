//
// Created by jorgealemangonzalez on 12/05/17.
//

#ifndef TJE_FRAMEWORK_2017_BULLETMANAGER_H
#define TJE_FRAMEWORK_2017_BULLETMANAGER_H

#include "framework.h"
#include "constants.h"

#define MAX_BULLETS 10000
class Bullet{
public:
    Vector3 position;       // Posición: V3
    Vector3 last_position;  // posición en el intervalo anterior
    Vector3 velocity;       // Velocidad: V3 (contiene tambien la dirección)
    float ttl;              // time to live, tiempo de vida que le queda
    float power;            // daño que hace la bala al colisionar
    UID author;             // quien ha disparado la bala
    std::string type;       // por si hay diferentes tipos de bala

    Bullet(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity, float ttl, float power,
           UID author, const std::string &type);


};

class BulletManager {
public:
    Bullet bullets_pool[MAX_BULLETS];
    unsigned int last_pos_pool;

    BulletManager();
    ~BulletManager();

    void removePosFromPool(unsigned int pos);
    void render();
    void update( float elapsed_time );
    void createBullet(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity, float ttl, float power,
                      UID author, const std::string &type);

};


#endif //TJE_FRAMEWORK_2017_BULLETMANAGER_H
