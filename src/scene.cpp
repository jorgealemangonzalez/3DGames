#include "scene.h"
#include "extra/textparser.h"
#include "game.h"

#define MIPMAP_DISABLE false

Scene* Scene::scene = NULL;
Scene::Scene() : debugMode(false) {
    root = new Entity();
    background = NULL;
    grid = NULL;

    debugPointsMesh = new Mesh();
    debugLinesMesh = new Mesh();
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

void Scene::addGrid(Entity * e) {
    Scene::getScene()->grid = e;
}

void Scene::render(Camera* camera) {
    if(background != NULL || grid != NULL){
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        if(background != NULL){
            Vector3 cubemapcenter = camera->eye;
            background->model.setIdentity().setTranslation(cubemapcenter.x, cubemapcenter.y, cubemapcenter.z);
            background->render(camera);
        }
        if(grid != NULL){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            grid->render(camera);
            glDisable(GL_BLEND);
        }
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
    root->render(camera);
    BulletManager::getManager()->render();
    if(debugMode){
        if(debugPointsMesh->vertices.size() > 0)
            debugPointsMesh->render(GL_POINT);
        if(debugLinesMesh->vertices.size() > 0)
            debugLinesMesh->render(GL_LINE);
    }

}

void Scene::loadScene(const char* filename) {
    TextParser t;
    if(!t.create(filename)){
        std::cout<<"File not found"<<std::endl;
        exit(0);
    }
    t.reset();

    std::map<std::string,Entity*> templates;  //Mesh info for position

    {   //Background
        EntityMesh* background = new EntityMesh();
        t.seek("*background");
        t.seek("*mesh");
        background->setMesh(t.getword());
        t.seek("*texture");
        std::string texture = t.getword();
        if(texture != "no") {   //Si tiene textura
            background->setTexture(texture);
        }
        //background->setPixelShader("polar_background.fs");
        Texture::Load(texture,MIPMAP_DISABLE);  //Load here to disable mipmaps ( cube lines )
        this->addBackground(background);
    }

    /*{   //Grid
        Mesh *plane = new Mesh();
        plane->createQuad(0,0,1000,1000);
        plane->uvs[0] = Vector2(10,10);
        plane->uvs[1] = Vector2(10,-10);
        plane->uvs[2] = Vector2(-10,-10);
        plane->uvs[3] = Vector2(-10,10);
        plane->uvs[4] = Vector2(10,10);
        plane->uvs[5] = Vector2(-10,-10);
        Mesh::s_Meshes["_grid"] = plane;
        EntityMesh* grid = new EntityMesh();
        grid->setMesh("_grid");
        t.seek("*grid");
        t.seek("*texture");
        std::string texture = t.getword();
        if(texture != "no") {
            grid->setTexture(texture);
        }
        this->addGrid(grid);
    }*/

    t.seek("*entities");
    int num_entities = t.getint();

    for(int i = 0 ; i < num_entities ; i++) {
        std::string name = t.getword();
        t.seek("*entity");
        std::string entityClass = t.getword();
        Entity *e;
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
            t.seek("*mesh");
            ec->setMesh(t.getword());
            t.seek("*texture");
            std::string texture = t.getword();
            if(texture != "no")   //Si tiene textura
                ec->setTexture(texture);
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
            e = ec;
        }else if(entityClass == "spawner"){
            EntitySpawner* es = new EntitySpawner();
            t.seek("*spawns");
            std::string spawnName = t.getword();
            EntityCollider* spawnEntity = (EntityCollider*)templates[spawnName];
            es->entitySpawned = spawnEntity->uid;
            t.seek("*spawntime");
            es->spawnTime = t.getfloat();
            e = es;
        }
        else{
            std::cout<<"Error reading entity class type for "<<name<<std::endl;
            exit(0);
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

        if(EntityCollider* ec = dynamic_cast<EntityCollider*>(clone)){
            EntityCollider::registerCollider(ec);
        }

        //HACK
        //TODO remove this
        if(entity_name == "fighter"){
            Game::instance->human->addControllableEntity(clone->uid);
            Game::instance->human->controlling_entity = clone->uid;
            Game::instance->enemy->aiController->setEntityFollow(clone->uid);

        }
        if(entity_name == "hunter"){
            Game::instance->enemy->addControllableEntity(clone->uid);
        }
        //FIN_HACK
        std::cout<<"Entity loaded: "<<entity_name<<"\n";
        std::cout<<"\t"<<clone->uid<<": "<<clone->getPosition().x<<" "<<clone->getPosition().y<<" "<<clone->getPosition().z<<"\n";
    }
}

void Scene::update(float elapsed_time) {

    this->root->update(elapsed_time);

    Camera* camera = Game::instance->camera;

    BulletManager::getManager()->update(elapsed_time);
    EntityCollider::checkCollisions();

    //Test collisions with island: needs low res sphere
    /*
    Vector3 dir = camera->center - camera->eye;
    Vector3 pos = camera->eye;
    EntityCollider* island = (EntityCollider*)s_templates["island"];

    if(island->testRayCollision(pos, dir, 1000000, collision)){
        EntityMesh* em = new EntityMesh();
        em->setMesh("sphere.ASE");
        em->model.setTranslation(collision.x, collision.y, collision.z);
        em->shaderDesc.fs = "color.fs";
        em->shaderDesc.vs = "color.vs";
        this->addToRoot(em);
    }*/
}

void Scene::addDebugPoint(Vector3 pos1) {}

void Scene::addDebugPoint(Vector3 pos1, Vector3 color) {}

void Scene::addDebugLine(Vector3 pos1, Vector3 pos2) {}

void Scene::addDebugLine(Vector3 pos1, Vector3 pos2, Vector3 color) {}