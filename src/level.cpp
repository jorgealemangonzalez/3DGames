//
// Created by jorgealemangonzalez on 26/04/17.
//

#include "level.h"
#include "extra/textparser.h"
#include "game.h"
#include "entity.h"
#include "scene.h"

Level::Level(){

}

void Level::createLevel1(Camera* camera){
    EntityCollider *plane = new EntityCollider();
    plane->setMesh("spitfire/spitfire.ASE");
    plane->texture = "spitfire/spitfire_color_spec.tga";
    plane->model.setTranslation(0.0, 500.0, 0.0);
    Scene::addToRoot(plane);

    //Extra planes
    for (int i = 0; i < 100; i++) {
        EntityCollider *p = new EntityCollider();
        p->setMesh("spitfire/spitfire.ASE");
        p->texture = "spitfire/spitfire_color_spec.tga";
        p->model.setTranslation(10.0 * (i%10), 500.0 + (i / 10) * 10.0, 10.0);
        Scene::addToRoot(p);
    }

    //Camera beside plane
    plane->followWithCamera(camera);
    //hide the cursor
    //TODO entitycollider call to methods

    EntityCollider *island = new EntityCollider();
    island->setMesh("island/island.ASE");
    island->texture = "island/island_color_luz.tga";
    Scene::addToRoot(island);
}

bool Level::load(const char* filename){
    TextParser t;
    if(!t.create(filename)){
        std::cout<<"File not found"<<std::endl;
        exit(0);
    }
    t.reset();
    t.seek("*entities");
    int num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++){
        std::string name = t.getword();
        t.seek("*entity");
        std::string entityClass = t.getword();
        EntityMesh* e;

        if(entityClass == "collider"){
            EntityCollider* ec = new EntityCollider();
            std::string colliderType = t.getword();
            if(colliderType == "static"){

                static_colliders.push_back(ec);
                printf("\n\nStatic\n\n");
            }else if(colliderType == "dynamic"){
                dynamic_colliders.push_back(ec);
            }else{
                std::cout<<"Error reading collider type in "<<name<<std::endl;
                exit(0);
            }
            e = ec;
        }else{
            std::cout<<"Error reading entity class type for "<<name<<std::endl;
            exit(0);
        }

        t.seek("*mesh");
        e->setMesh(t.getword());
        t.seek("*texture");
        char* texture = t.getword();
        if(strcmp(texture,"no") != 0)   //Si tiene textura
            e->texture = texture;
        s_templates[name] = e;
    }

    t.seek("*scene");
    this->name = t.getword();
    num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++){
        t.seek("*entity");
        std::string entity_name = t.getword();
        EntityMesh* e = s_templates[entity_name];
        if(!e){
            std::cout<<"Entity in file not found: "<<entity_name<<std::endl;
            continue;
        }

        EntityMesh* clone = new EntityMesh();
        *clone = *e;//Copiar todas las variables ( Si hay punteros la has cagado porque se copiara la referencia
        t.seek("*position");
        clone->model.setTranslation(t.getint(),t.getint(),t.getint());
        Scene::addToRoot(clone);
        //hack cutre no te enfades:
        if(entity_name == "plane"){
            player->setMyEntity(clone);
        }
        //fin hack
        printf("Entity loaded: %s",&entity_name);
    }
}

void Level::update(float elapsed_time) {
    Camera* camera = Game::instance->camera;
    Vector3 dir = camera->center - camera->eye;
    Vector3 pos = camera->eye;

    EntityCollider* island = (EntityCollider*)s_templates["island"];

    Vector3 collision;
    if(island->testCollision(pos, dir, 1000000, collision) && false){
        EntityMesh* em = new EntityMesh();
        em->setMesh("sphere.ASE");
        em->model.setTranslation(collision.x, collision.y, collision.z);
        em->shaderDesc.fs = "color.fs";
        em->shaderDesc.vs = "color.vs";
        root->addChild(em);
    }

    for(int i = 0 ; i < dynamic_colliders.size(); ++i){
        dynamic_colliders[i]->setTransform();
        Vector3 dinamic_pos = dynamic_colliders[i]->getGlobalModel().getTranslationOnly();
        for(int st = 0 ; st < static_colliders.size(); ++st){
            if(static_colliders[st]->testCollision(dinamic_pos,20.0f, collision)) {
                dynamic_colliders[i]->onCollision(static_colliders[st]);
            }
        }
        for(int j = i+1 ; j < dynamic_colliders.size(); ++j){
            if(dynamic_colliders[j]->testCollision(dinamic_pos,20.0f, collision))
                dynamic_colliders[i]->onCollision(dynamic_colliders[j]);
        }
    }
}