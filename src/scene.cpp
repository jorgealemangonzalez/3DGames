#include "scene.h"
#include "extra/textparser.h"
#include "game.h"
#include "entity.h"

#define MIPMAP_DISABLE false

Scene* Scene::scene = NULL;
Scene::Scene() {
    root = new Entity();
    background = NULL;
    grid = NULL;

    root_background = new Entity();
    EntityMesh* planet_background = new EntityMesh();
    planet_background->mesh = "planet/sphere.ASE";
    planet_background->texture = "planet/planet_surface.tga";
    planet_background->model.setTranslation(400,-200,500);
    root_background->addChild(planet_background);
    EntityMesh* planet_background2 = new EntityMesh();
    planet_background2->mesh = "planet/sphere.ASE";
    planet_background2->texture = "planet/purple_planet.tga";
    planet_background2->model.setTranslation(-200,20,200);
    root_background->addChild(planet_background2);
    EntityMesh* planet_background3 = new EntityMesh();
    planet_background3->mesh = "planet/sphere.ASE";
    planet_background3->texture = "planet/craters.tga";
    planet_background3->model.setTranslation(200,0,-300);
    root_background->addChild(planet_background3);

    Game* game = Game::instance;
    camera2d = new Camera();
    camera2d->setOrthographic(0, game->window_width, 0, game->window_height, 0, 1);
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
    root_background->model.setIdentity().setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
    root_background->render(camera);
    glClear(GL_DEPTH_BUFFER_BIT);

    root->render(camera);
    BulletManager::getManager()->render();

    if(_DEBUG_){
        debugPointsMesh->clear();
        debugLinesMesh->clear();
        for(auto &entry : Entity::s_entities){
            if(entry.second->stats.selectable){
                Vector4 color(0,0,1,1);
                if(entry.second->stats.selected)
                    color = Vector4(0,1,0,1);
                Vector3 pos = camera->project(entry.second->getPosition(), Game::instance->window_width, Game::instance->window_height);
                addDebugPoint(pos, color);
                if(EntityMesh* em = dynamic_cast<EntityMesh*>(entry.second)) {
                    double meshRadius = Mesh::Load(em->mesh)->info.radius;
                    double radius = camera->getProjectScale(entry.second->getPosition(), meshRadius) / 1200.;
                    addDebugLine(Vector3(pos.x - radius, pos.y - radius, pos.z), Vector3(pos.x + radius, pos.y - radius, pos.z), color);
                    addDebugLine(Vector3(pos.x + radius, pos.y - radius, pos.z), Vector3(pos.x + radius, pos.y + radius, pos.z), color);
                    addDebugLine(Vector3(pos.x + radius, pos.y + radius, pos.z), Vector3(pos.x - radius, pos.y + radius, pos.z), color);
                    addDebugLine(Vector3(pos.x - radius, pos.y + radius, pos.z), Vector3(pos.x - radius, pos.y - radius, pos.z), color);
                }
            }
        }
        camera2d->set();
        glDisable(GL_DEPTH_TEST);
        if(debugPointsMesh->vertices.size())
            debugPointsMesh->render(GL_POINTS);
        if(debugLinesMesh->vertices.size())
            debugLinesMesh->render(GL_LINES);
        glEnable(GL_DEPTH_TEST);
        camera->set();
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
            if(munition == "yes") {
                //it has munition
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
            t.seek("*stats");
            Stats stats;
            stats.movable = (strcmp(t.getword(),"true") == 0);
            stats.has_hp = (strcmp(t.getword(),"true") == 0);
            stats.has_ttl = (strcmp(t.getword(),"true") == 0);
            stats.selectable = (strcmp(t.getword(),"true") == 0);
            stats.hp = t.getint();
            stats.ttl = t.getfloat();
            stats.team = t.getword();
            es->statsSpawned = stats;
            e = es;
        }
        else{
            std::cout<<"Error reading entity class type for "<<name<<std::endl;
            exit(0);
        }

        templates[name] = e;
        e->name = name;
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

        t.seek("*stats");
        Stats stats;
        stats.movable = (strcmp(t.getword(),"true") == 0);
        stats.has_hp = (strcmp(t.getword(),"true") == 0);
        stats.has_ttl = (strcmp(t.getword(),"true") == 0);
        stats.selectable = (strcmp(t.getword(),"true") == 0);
        stats.hp = t.getint();
        stats.ttl = t.getfloat();
        stats.team = t.getword();
        clone->stats = stats;
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

void Scene::addDebugPoint(Vector3 pos1) {
    debugPointsMesh->vertices.push_back(pos1);
    debugPointsMesh->colors.push_back(Vector4(1,1,1,1));
}

void Scene::addDebugPoint(Vector3 pos1, Vector4 color) {
    debugPointsMesh->vertices.push_back(pos1);
    debugPointsMesh->colors.push_back(color);
}

void Scene::addDebugLine(Vector3 pos1, Vector3 pos2) {
    debugLinesMesh->vertices.push_back(pos1);
    debugLinesMesh->vertices.push_back(pos2);
    debugLinesMesh->colors.push_back(Vector4(1,1,1,1));
    debugLinesMesh->colors.push_back(Vector4(1,1,1,1));
}

void Scene::addDebugLine(Vector3 pos1, Vector3 pos2, Vector4 color) {
    debugLinesMesh->vertices.push_back(pos1);
    debugLinesMesh->vertices.push_back(pos2);
    debugLinesMesh->colors.push_back(color);
    debugLinesMesh->colors.push_back(color);
}