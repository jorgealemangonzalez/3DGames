//
// Created by jorgealemangonzalez on 24/06/17.
//

#include <iostream>
#include "MusicManager.h"
#include "framework.h"
#include "game.h"

#define SOUND_POOL_SIZE 250
sf::Music MusicManager::ambientTense;
sf::SoundBuffer MusicManager::bulletBuffer;
sf::SoundBuffer MusicManager::explosionBuffer;
sf::SoundBuffer MusicManager::enemyDownBuffer;

sf::Sound* sound_pool[SOUND_POOL_SIZE];
unsigned pool_index = 0;

void MusicManager::init() {
    if(!ambientTense.openFromFile("../data/sounds/space.wav"))
        std::cout<<"ERROR OPENING ../data/sounds/space.wav\n";
    ambientTense.setLoop(true);

    if(!bulletBuffer.loadFromFile("../data/sounds/laser_weapon_short.wav"))
        std::cout<<"ERROR OPENING ../data/sounds/laser_weapon_short.wav\n";

    if(!explosionBuffer.loadFromFile("../data/sounds/explosion_short.wav"))
        std::cout<<"ERROR OPENING ../data/sounds/explosion_short.wav\n";

    if(!enemyDownBuffer.loadFromFile("../data/sounds/enemy_down.ogg"))
        std::cout<<"ERROR OPENING ../data/sounds/enemy_down.wav\n";

}

void MusicManager::delete_pool(){
    for(sf::Sound* s: sound_pool){
        if(s != NULL){
            s->stop();
            delete s;
        }

    }
}

void MusicManager::update() {

    Camera* c = Game::instance->camera;
    Vector3 listenerPos = c->eye;
    Vector3 orientation = c->center - c->eye;
    orientation.normalize();
    sf::Listener::setPosition(listenerPos.toMusicVector());
    sf::Listener::setDirection(orientation.toMusicVector());
    sf::Listener::setUpVector(c->up.toMusicVector());
    sf::Listener::setGlobalVolume(50.f);
}


void MusicManager::playPoolSound(sf::Sound* sound, Vector3 position){
    if(sound_pool[pool_index] != NULL) {
        sound_pool[pool_index]->stop();
        delete sound_pool[pool_index]; //Delete previus sound
    }
    sound_pool[pool_index] = sound;
    sound->setPosition(position.toMusicVector());
    sound->setMinDistance(5.f);
    float rand = (random() % 100000) / 100000.0f;
    sound->setPitch(rand/2.0+1.0);
    sound->play();

    pool_index = (pool_index+1)%SOUND_POOL_SIZE;
}


void MusicManager::playBullet(Vector3 position){
    sf::Sound* bullet = new sf::Sound();
    bullet->setAttenuation(0.1);
    bullet->setBuffer(bulletBuffer);
    playPoolSound(bullet,position);

}

void MusicManager::playExplosion(Vector3 position){
    sf::Sound* explosion = new sf::Sound();
    explosion->setBuffer(explosionBuffer);
    explosion->setAttenuation(0.1);
    playPoolSound(explosion,position);
}

void MusicManager::playEnemyDown(Vector3 position){
    sf::Sound* enemyDown = new sf::Sound();
    enemyDown->setBuffer(enemyDownBuffer);
    enemyDown->setAttenuation(0.01);
    enemyDown->setPitch(0.3);
    playPoolSound(enemyDown,position);
}
