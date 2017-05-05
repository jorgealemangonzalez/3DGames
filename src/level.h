//
// Created by jorgealemangonzalez on 26/04/17.
//

#ifndef TJE_FRAMEWORK_2017_LEVEL_H
#define TJE_FRAMEWORK_2017_LEVEL_H

#include <map>
#include "entity.h"
#include "player.h"

class Level {
public:
    Level();

    Entity* root;
    Player* player;
    std::string name;
    std::vector<EntityCollider*> static_colliders;   // NO se mueven, así que no hay colisiones entre ellas
    std::vector<EntityCollider*> dynamic_colliders;  // Pueden colisionar entre ellas y con static_colliders
    std::map<std::string,EntityMesh*> s_templates;

    void createLevel1(Entity* root, Camera* camera);

    bool load(const char* filename, Entity* root);

    void render();
    void update(float elapsed_time);
};


#endif //TJE_FRAMEWORK_2017_LEVEL_H
