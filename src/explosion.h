//
// Created by jorgealemangonzalez on 24/05/17.
//

#ifndef TJE_FRAMEWORK_2017_EXPLOSION_H
#define TJE_FRAMEWORK_2017_EXPLOSION_H

#include<vector>
#include "framework.h"
#include "camera.h"

class Explosion {
public:
    static std::vector<Explosion*> explosions;

    static void renderAll(Camera* camera);
    static void updateAll(float elapsed_time);
    static void generateExplosion(Vector3 exploding_point);
    Vector3 exploding_point;
    float time;
    float lifetime;

    Explosion();
    ~Explosion();



};


#endif //TJE_FRAMEWORK_2017_EXPLOSION_H
