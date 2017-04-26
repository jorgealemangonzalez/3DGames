//
// Created by jorgealemangonzalez on 26/04/17.
//

#ifndef TJE_FRAMEWORK_2017_LEVEL_H
#define TJE_FRAMEWORK_2017_LEVEL_H

#include <map>
#include "entity.h"

class Level {
public:
    std::string name;

    Level();
    std::map<std::string,EntityMesh*> s_templates;

    void createLevel1(Entity* root, Camera* camera);

    bool load(const char* filename, Entity* root);
};


#endif //TJE_FRAMEWORK_2017_LEVEL_H
