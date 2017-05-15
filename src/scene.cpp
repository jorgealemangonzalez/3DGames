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
        std::string texture = t.getword();
        if(texture != "no")   //Si tiene textura
            background->setTexture(texture);
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
            EntityCollider *ec;
            std::string colliderType = t.getword();
            if (colliderType == "static") {
                ec = new EntityCollider(false);
            } else if (colliderType == "dynamic") {
                ec = new EntityCollider(true);
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
        std::string texture = t.getword();
        if(texture != "no")   //Si tiene textura
            e->setTexture(texture);
        t.seek("*munition");
        std::string munition = t.getword();
        if(munition != "no") {
            //It has munition
            t.seek("*generate");
            float x = t.getint();
            float y = t.getint();
            float z = t.getint();
            Vector3 generatePos = Vector3(x,y,z);
            std::cout<<"Generate Pos: "+generatePos.toString()+"\n";
            if (munition == "ray") {
                //Ray munition
                std::cout<<"RAY: ";
                t.seek("*color");
                Vector3 color = Vector3(t.getint(),t.getint(),t.getint());
                std::cout<<"color "+color.toString()+"\n";
            } else if (munition == "mesh") {
                //Mesh munition
                std::cout<<"MESH_MUNITION: ";
                t.seek("*mesh");

                std::string munition_mesh = t.getword();
                t.seek("*texture");
                std::string munition_tga = t.getword();

                std::cout<<" mesh "<<munition_mesh<<" tga "<<munition_tga<<"\n";

            }
        }
        templates[name] = e;
    }

    t.seek("*scene");
    this->name = t.getword();
    num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++){
        t.seek("*entity");
        std::string entity_name = t.getword();
        auto *e = templates[entity_name];
        if(!e){
            std::cout<<"Entity in file not found: "<<entity_name<<std::endl;
            continue;
        }

        auto* clone = e->clone();
        t.seek("*position");
        clone->model.setTranslation(t.getint(),t.getint(),t.getint());
        this->addToRoot(clone);

        try {
            EntityCollider* ec = (EntityCollider*) clone;
            EntityCollider::registerCollider(ec);
        } catch(...) {}

        //HACK
        //TODO remove this
        if(entity_name == "fighter" || entity_name == "hunter"){
            player->addControllableEntity(clone->uid);
        }
        //FIN_HACK
        std::cout<<"Entity loaded: "<<entity_name<<"\n";
        std::cout<<"\t"<<clone->uid<<": "<<clone->getPosition().x<<" "<<clone->getPosition().y<<" "<<clone->getPosition().z<<"\n";
        Entity* eee = Entity::getEntity(clone->uid);
        std::cout<<"\t"<<eee->uid<<": "<<eee->getPosition().x<<" "<<eee->getPosition().y<<" "<<eee->getPosition().z<<"\n";
    }
}

void Scene::update(float elapsed_time) {

    this->root->update(elapsed_time);

    Camera* camera = Game::instance->camera;

    EntityCollider::checkCollisions();

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
}