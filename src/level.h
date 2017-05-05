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
    Entity* root;
    Player* player;

    std::string name;

    Level();
    std::map<std::string,EntityMesh*> s_templates;

    void createLevel1(Entity* root, Camera* camera);

    bool load(const char* filename, Entity* root);

    void render();
    void update(float elapsed_time);
};


#endif //TJE_FRAMEWORK_2017_LEVEL_H
