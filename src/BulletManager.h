//
// Created by jorgealemangonzalez on 12/05/17.
//

#ifndef TJE_FRAMEWORK_2017_BULLETMANAGER_H
#define TJE_FRAMEWORK_2017_BULLETMANAGER_H

#include "framework.h"
#include "constants.h"
#include <string>
#include <sstream>
#include <iostream>

#define MAX_BULLETS 100
class Bullet{
public:
    Vector3 position;       // Posición: V3
    Vector3 last_position;  // posición en el intervalo anterior
    Vector3 velocity;       // Velocidad: V3 (contiene tambien la dirección)
    float ttl;              // time to live, tiempo de vida que le queda
    float power;            // daño que hace la bala al colisionar
    std::string type;       // por si hay diferentes tipos de bala //TODO crear valas que llegan mas lejos o cerca
    std::string team;       //Equipo quien la dispara


    Bullet();
    ~Bullet();
    void set(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity, float ttl, float power,
             std::string team, const std::string &type);


};

class BulletManager {
    BulletManager();

    static BulletManager* manager;  //Singleton class
public:
    ~BulletManager();
    Bullet bullets_pool[MAX_BULLETS];   // La pool se inicializa entera de colpe.
                                        // Luego no se crean ni destruyen bullets, para ello set.
    unsigned int last_pos_pool;

    void removePosFromPool(unsigned int pos);
    void render();
    void update( float elapsed_time );
    void createBullet(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity, float ttl, float power,
                      UID author, const std::string &type);

    static BulletManager* getManager(); //Obtain the instance

};


#endif //TJE_FRAMEWORK_2017_BULLETMANAGER_H
