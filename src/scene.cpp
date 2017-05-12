#include "scene.h"
#include "extra/textparser.h"
#include "game.h"

Scene* Scene::scene = NULL;
Scene::Scene(){
    root = new Entity();
    background = NULL;
}
Scene::~Scene(){

}

Scene* Scene::getScene() {
    if(scene == NULL){
        scene = new Scene();
    }
    return scene;
}

void Scene::addToRoot(Entity *e) {
    Scene::getScene()->root->addChild(e);
}

void Scene::addBackground(Entity *e) {
    Scene::getScene()->background = e;
}

void Scene::render(Camera* camera) {
    if(background != NULL){
        glDisable(GL_DEPTH_TEST);
        Vector3 cubemapcenter = camera->eye;
        background->model.setIdentity().setTranslation(cubemapcenter.x, cubemapcenter.y, cubemapcenter.z);
        background->render(camera);
        glEnable(GL_DEPTH_TEST);
    }
    root->render(camera);
}

void Scene::loadScene(const char* filename) {
    TextParser t;
    if(!t.create(filename)){
        std::cout<<"File not found"<<std::endl;
        exit(0);
    }
    t.reset();

    std::map<std::string,EntityMesh*> templates;  //Mesh info for position

    {   //Background
        EntityMesh* background = new EntityMesh();
        t.seek("*background");
        t.seek("*mesh");
        background->setMesh(t.getword());
        t.seek("*texture");
        char* texture = t.getword();
        if(strcmp(texture,"no") != 0)   //Si tiene textura
            background->texture = texture;
        this->addBackground(background);
    }

    t.seek("*entities");
    int num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++) {
        std::string name = t.getword();
        t.seek("*entity");
        std::string entityClass = t.getword();
        EntityMesh *e;
        std::cout << entityClass << "\n";

        if (entityClass == "collider") {
            EntityCollider *ec = new EntityCollider();
            std::string colliderType = t.getword();
            if (colliderType == "static") {

                static_colliders.push_back(ec);
                printf("\n\nStatic\n\n");
            } else if (colliderType == "dynamic") {
                dynamic_colliders.push_back(ec);
            } else {
                std::cout << "Error reading collider type in " << name << std::endl;
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
        templates[name] = e;
    }

    t.seek("*scene");
    this->name = t.getword();
    num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++){
        t.seek("*entity");
        std::string entity_name = t.getword();
        EntityMesh* e = templates[entity_name];
        if(!e){
            std::cout<<"Entity in file not found: "<<entity_name<<std::endl;
            continue;
        }

        EntityMesh* clone = new EntityMesh();
        *clone = *e;//Copiar todas las variables ( Si hay punteros la has cagado porque se copiara la referencia
        t.seek("*position");
        clone->model.setTranslation(t.getint(),t.getint(),t.getint());
        this->addToRoot(clone);

        //hack temporal para controlar el avión:
        if(entity_name == "plane"){
            player->setMyEntity(clone);
        }
        //fin hack
        printf("Entity loaded: %s",&entity_name);
    }
}

void Scene::update(float elapsed_time) {

    this->root->update(elapsed_time);

    Camera* camera = Game::instance->camera;
    Vector3 collision;

    //Test collisions with island: needs low res sphere
    /*
    Vector3 dir = camera->center - camera->eye;
    Vector3 pos = camera->eye;
    EntityCollider* island = (EntityCollider*)s_templates["island"];

    if(island->testCollision(pos, dir, 1000000, collision)){
        EntityMesh* em = new EntityMesh();
        em->setMesh("sphere.ASE");
        em->model.setTranslation(collision.x, collision.y, collision.z);
        em->shaderDesc.fs = "color.fs";
        em->shaderDesc.vs = "color.vs";
        this->addToRoot(em);
    }*/

    //Test every possible collision
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