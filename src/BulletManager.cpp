//
// Created by jorgealemangonzalez on 12/05/17.
//

#include "BulletManager.h"
#include "entity.h"
#include "explosion.h"
#include "MusicManager.h"
#include "utils.h"
#include "mesh.h"

BulletManager* BulletManager::manager;

Bullet::Bullet() {}
Bullet::~Bullet() {}

void Bullet::set(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity, float ttl, float power,
                 std::string team, const std::string &type) {
    this->position = position;
    this->last_position = last_position;
    this->velocity = velocity;
    this->ttl = ttl;
    this->power = power;
    this->team = team;
    this->type = type;
}


BulletManager::BulletManager(){
    Bullet bullets_pool[MAX_BULLETS];
    last_pos_pool = 0;
}
BulletManager::~BulletManager(){

}

void BulletManager::removePosFromPool(unsigned int pos){
    if(pos < last_pos_pool){
        Bullet &fb = bullets_pool[pos];
        if( pos != last_pos_pool-1){
            bullets_pool[pos] = bullets_pool[last_pos_pool-1];
        }
        last_pos_pool--;
    }
}


void BulletManager::update(float elapsed_time) {
    float now = getTime();

    for(unsigned int i = 0 ; i < last_pos_pool; ++i){
        Bullet &b = bullets_pool[i];
        b.ttl -=elapsed_time;
        if(b.ttl < 0.0 ){
            removePosFromPool(i);
            continue;
        }
        b.last_position = b.position;
        b.position = b.position + b.velocity*elapsed_time;
        bool hit = false;
        for(unsigned int i = 0 ; i < EntityCollider::dynamic_colliders.size(); ++i){
            EntityCollider* e = (EntityCollider*)EntityCollider::getEntity(EntityCollider::dynamic_colliders[i]);
            float radius = Mesh::Load(e->mesh)->info.radius;
            Vector3 collision_point;
            if(e->getPosition().distance(b.position) < radius && e->testRayCollision(b.position, (b.last_position - b.position).normalize(), 50.0f, collision_point)){
                if(e->stats.team != b.team) {
                    e->onCollision(&b);
                    Explosion::generateExplosion(collision_point);
                }else continue;

                hit = true;
                break;
            }
        }
        if(hit){
            removePosFromPool(i);
            i--;
            continue;
        }

        for(unsigned int i = 0 ; i < EntityCollider::static_colliders.size(); ++i){
            EntityCollider* e = (EntityCollider*)EntityCollider::getEntity(EntityCollider::static_colliders[i]);
            float radius = Mesh::Load(e->mesh)->info.radius;
            Vector3 collision_point;
            if(e->getPosition().distance(b.position) < radius && e->testRayCollision(b.position, (b.last_position - b.position).normalize(), 50.0f, collision_point)){
                if(e->stats.team != b.team) {
                    e->onCollision(&b);
                    Explosion::generateExplosion(collision_point);
                }else continue;
                hit = true;
                break;
            }
        }

        if(hit){
            removePosFromPool(i);
            i--;
            continue;
        }
    }

    if(debugMode)
        std::cout<<"BULLET TIME "<<getTime() - now<<"\n";
}

void BulletManager::render() {
    //TODO CREATE MESH WITH PREVIUS_POS AND POS OF ALL BULLETS
    //TODO RENDER THIS MESH WITH LINES 2 BY 2
    long before = SDL_GetTicks();
    if(last_pos_pool) {
        Mesh m;
        for (unsigned int i = 0; i < last_pos_pool; ++i) {
            m.vertices.push_back(bullets_pool[i].last_position);
            m.vertices.push_back(bullets_pool[i].position);
        }
        m.render(GL_LINES);
    }
    long after = SDL_GetTicks();
}

void BulletManager::createBullet(const Vector3 &position, const Vector3 &last_position, const Vector3 &velocity,
                                 float ttl, float power, UID author, const std::string &type) {
    MusicManager::playBullet(position);
    if(last_pos_pool >= MAX_BULLETS){
        unsigned int mini = -1;
        float minttl = 10000000000;
        for(unsigned int i = 0 ; i < last_pos_pool ; ++i)
            if(minttl > bullets_pool[i].ttl){
                mini = i;
                minttl = bullets_pool[i].ttl;
            }
        removePosFromPool(mini);
    }

    bullets_pool[last_pos_pool++].set(position,last_position,velocity,ttl,power,Entity::getEntity(author)->stats.team,type);
}

BulletManager* BulletManager::getManager() {
    if(!manager){
        manager = new BulletManager();
    }
    return manager;
}