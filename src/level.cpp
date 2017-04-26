//
// Created by jorgealemangonzalez on 26/04/17.
//

#include "level.h"
#include "extra/textparser.h"

Level::Level(){

}

void Level::createLevel1(Entity* root, Camera* camera){
    EntityMesh *plane = new EntityMesh();
    plane->mesh = "spitfire/spitfire.ASE";
    plane->texture = "spitfire/spitfire_color_spec.tga";
    plane->model.setTranslation(0.0, 500.0, 0.0);
    root->addChild(plane);

    //Extra planes
    for (int i = 0; i < 100; i++) {
        EntityMesh *p = new EntityMesh();
        p->mesh = "spitfire/spitfire.ASE";
        p->texture = "spitfire/spitfire_color_spec.tga";
        p->model.setTranslation(10.0 * (i%10), 500.0 + (i / 10) * 10.0, 10.0);
        root->addChild(p);
    }

    //Camera beside plane
    plane->followWithCamera(camera);
    //hide the cursor

    EntityMesh *island = new EntityMesh();
    island->mesh = "island/island.ASE";
    island->texture = "island/island_color_luz.tga";
    root->addChild(island);
}

bool Level::load(const char* filename, Entity* root){
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
        EntityMesh* e = new EntityMesh();
        e->mesh = t.getword();
        e->texture = t.getword();

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
        clone->model.setTranslation(t.getint(),t.getint(),t.getint());
        root->addChild(clone);
        
    }

}