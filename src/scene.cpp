#include "scene.h"
#include "extra/textparser.h"
#include "game.h"
#include "entity.h"
#include "mesh.h"

#define MIPMAP_DISABLE false

Scene* Scene::scene = NULL;
Scene::Scene() {
    root = new Entity();
    root->name = "root";
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
}
Scene::~Scene(){
    delete root;
    delete background;
    delete grid;

    delete root_background;
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
}

void Scene::loadScene(const char* filename) {
    TextParser t;
    if (!t.create(filename)) {
        std::cout << "File not found" << std::endl;
        exit(0);
    }
    t.reset();

    std::map<std::string, Entity *> templates;  //Mesh info for position

    {   //Background
        EntityMesh *background = new EntityMesh();
        t.seek("*background");
        t.seek("*mesh");
        background->setMesh(t.getword());
        t.seek("*texture");
        std::string texture = t.getword();
        if (texture != "no") {   //Si tiene textura
            background->setTexture(texture);
        }
        //background->setPixelShader("polar_background.fs");
        Texture::Load(texture, MIPMAP_DISABLE);  //Load here to disable mipmaps ( cube lines )
        this->addBackground(background);
    }

    t.seek("*entities");
    int num_entities = t.getint();

    for (int i = 0; i < num_entities; i++) {
        std::string name = t.getword();
        t.seek("*entity");
        std::string entityClass = t.getword();
        Entity *e;
        std::cout << entityClass << "\n";

        if (entityClass == "collider" || entityClass == "fighter" || entityClass == "station" || entityClass == "asteroid") {
            EntityCollider *ec;
            std::string colliderType;
            if(entityClass == "asteroid")
                colliderType = "static";
            else
                colliderType = t.getword();
            bool dynamic;
            if (colliderType == "static") {
                dynamic = false;
            } else if (colliderType == "dynamic") {
                dynamic = true;
            } else {
                std::cout << "Error reading collider type in " << name << std::endl;
                exit(0);
            }
            if (entityClass == "fighter")
                ec = new EntityFighter(dynamic);
            else if (entityClass == "station")
                ec = new EntityStation(dynamic);
            else if(entityClass == "asteroid") {
                ec = new EntityAsteroid(dynamic);
            }
            else
                ec = new EntityCollider(dynamic);
            t.seek("*mesh");
            ec->setMesh(t.getword());
            t.seek("*texture");
            std::string texture = t.getword();
            if (texture != "no")   //Si tiene textura
                ec->setTexture(texture);
            e = ec;
        } else if (entityClass == "spawner") {
            EntitySpawner *es = new EntitySpawner();
            t.seek("*spawns");
            std::string spawnName = t.getword();
            EntityCollider *spawnEntity = (EntityCollider *) templates[spawnName];
            es->entitySpawned = spawnEntity->uid;
            t.seek("*spawntime");
            es->spawnTime = t.getfloat();
            t.seek("*stats");
            Stats stats;
            stats.movable = (strcmp(t.getword(), "true") == 0);
            stats.has_hp = (strcmp(t.getword(), "true") == 0);
            stats.has_ttl = (strcmp(t.getword(), "true") == 0);
            stats.selectable = (strcmp(t.getword(), "true") == 0);
            bool winning = (strcmp(t.getword(), "true") == 0);
            stats.maxhp = t.getint();
            stats.hp = stats.maxhp;
            stats.ttl = t.getfloat();
            stats.range = t.getfloat();
            stats.maxvel = t.getint();
            stats.team = t.getword();
            es->statsSpawned = stats;
            e = es;
        } else {
            std::cout << "Error reading entity class type for " << name << std::endl;
            exit(0);
        }
        e->stats.isTemplate = true;
        templates[name] = e;
        e->name = name;
    }

    t.seek("*scene");
    this->name = t.getword();
    num_entities = t.getint();

    for (int i = 0; i < num_entities; i++) {
        t.seek("*entity");
        std::string entity_name = t.getword();
        auto *e = templates[entity_name];
        if (!e) {
            std::cout << "Entity in file not found: " << entity_name << std::endl;
            continue;
        }

        auto *clone = e->clone();
        t.seek("*position");
        int x, y, z;
        x = t.getint();
        y = t.getint();
        z = t.getint();
        clone->model.setTranslation(x, y, z);
        if (!dynamic_cast<EntitySpawner *>(clone)) {
            t.seek("*stats");
            Stats stats;
            stats.movable = (strcmp(t.getword(), "true") == 0);
            stats.has_hp = (strcmp(t.getword(), "true") == 0);
            stats.has_ttl = (strcmp(t.getword(), "true") == 0);
            stats.selectable = (strcmp(t.getword(), "true") == 0);
            stats.mantainAlive = (strcmp(t.getword(), "true") == 0);
            stats.maxhp = t.getint();
            stats.hp = stats.maxhp;
            stats.ttl = t.getfloat();
            stats.range = t.getfloat();
            stats.maxvel = t.getint();
            stats.team = t.getword();
            clone->stats = stats;

            if(stats.team == HUMAN_TEAM){
                Human* human = Game::instance->human;
                human->addControllableEntity(clone->uid);
                if(stats.mantainAlive)
                    human->maintainAliveEntities.push_back(clone->uid);
            }else if(stats.team == ENEMY_TEAM){
                Enemy* enemy = Game::instance->enemy;
                enemy->addControllableEntity(clone->uid);
                if(stats.mantainAlive)
                    enemy->maintainAliveEntities.push_back(clone->uid);
            }
        }
        this->addToRoot(clone);



        if (EntityCollider *ec = dynamic_cast<EntityCollider *>(clone)) {
            EntityCollider::registerCollider(ec);
        }

        std::cout << "Entity loaded: " << entity_name << "\n";
        std::cout << "\t" << clone->uid << ": " << clone->getPosition().x << " " << clone->getPosition().y << " "
                  << clone->getPosition().z << "\n";
    }

    //GENERATE RANDOM ASTEROIDS

    //FIRST:: get full bounding box
    Vector3 boxMin(2147483647,2147483647,2147483647), boxMax;
    auto entities = Entity::s_entities;
    for(auto it = entities.begin(); it != entities.end(); ++it ){
        if(it->second->stats.isTemplate)
            continue;

        Vector3 entityPos = it->second->getPosition();
        boxMax.x = MAX(boxMax.x, entityPos.x);
        boxMax.y = MAX(boxMax.y, entityPos.y);
        boxMax.z = MAX(boxMax.z, entityPos.z);

        boxMin.x = MIN(boxMin.x, entityPos.x);
        boxMin.y = MIN(boxMin.y, entityPos.y);
        boxMin.z = MIN(boxMin.z, entityPos.z);
    }

    //SECOND:: generate random asteroids
    EntityAsteroid asteroid_peq(false);
    EntityAsteroid asteroid_med(false);
    asteroid_peq.setMesh("asteroides/asteroide3.ASE");
    asteroid_peq.setTexture("asteroides/asteroide.tga");;
    asteroid_med.setMesh("asteroides/asteroide2.ASE");
    asteroid_med.setTexture("asteroides/asteroide.tga");
    float asteroidRadius_peq = Mesh::Load(asteroid_peq.mesh)->info.radius;
    float asteroidRadius_med = Mesh::Load(asteroid_med.mesh)->info.radius;


    for(int try_ = 0 ; try_ < 200 ; ++try_){ //
        float asteroidRadius;
        EntityAsteroid* asteroid;
        if(try_ % 2 == 0){
            asteroid= &asteroid_peq;
            asteroidRadius = asteroidRadius_peq;
        }else{
            asteroid= &asteroid_med;
            asteroidRadius = asteroidRadius_med;
        }

        Vector3 rpos;
        rpos.random(boxMin, boxMax);
        std::map<UID,Entity*>::iterator it;
        for( it = entities.begin(); it!= entities.end(); ++it){
            if(rpos.distance(it->second->getPosition()) < asteroidRadius*3)
                break;
        }

        if(it == entities.end()){
            std::cout<<"ASTEROIDE RANDOM";
            EntityAsteroid* as = asteroid->clone();
            as->model.traslate(rpos.x,rpos.y,rpos.z);
            this->addToRoot(as);
        }

    }


}

void Scene::update(float elapsed_time) {
    Entity::destroy_entities_to_destroy();
    this->root->update(elapsed_time);

    Camera* camera = Game::instance->camera;

    BulletManager::getManager()->update(elapsed_time);
    EntityCollider::checkCollisions(elapsed_time);
}

void Scene::updateGUI() {
    this->root->updateGUI();
}
