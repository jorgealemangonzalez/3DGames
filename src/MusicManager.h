//
// Created by jorgealemangonzalez on 24/06/17.
//

#ifndef TJE_FRAMEWORK_2017_MUSICMANAGER_H
#define TJE_FRAMEWORK_2017_MUSICMANAGER_H
#include <SFML/Audio.hpp>
#include "framework.h"

class MusicManager {
public:
    static sf::Music ambientTense;
    static sf::SoundBuffer bulletBuffer;
    static sf::SoundBuffer explosionBuffer;
    static sf::SoundBuffer enemyDownBuffer;

    static void stop_pool();
    static void init();
    static void update();
    static void playBullet(Vector3 position);
    static void playExplosion(Vector3 position);
    static void playEnemyDown(Vector3 position);
    static void playPoolSound(sf::Sound* sound, Vector3 position);
};


#endif //TJE_FRAMEWORK_2017_MUSICMANAGER_H
