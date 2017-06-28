//
// Created by jorgealemangonzalez on 24/05/17.
//

#include "explosion.h"
#include "mesh.h"
#include "entity.h"
#include "MusicManager.h"

std::vector<Explosion*> Explosion::explosions;

Explosion::Explosion() {
    time = 0;
    lifetime = 1.0; //Dura mas de lo que deberia -.-
}

Explosion::~Explosion() {

}

void Explosion::generateExplosion(Vector3 exploding_point) {
    Explosion* e = new Explosion();
    e->exploding_point = exploding_point;
    MusicManager::playExplosion(exploding_point);
    Explosion::explosions.push_back(e);
}

void Explosion::renderAll(Camera *camera) {
    if(!explosions.size()) {  //Nothing to render
        return;
    }
    Mesh m;
    for(Explosion* e : explosions){
        //m.createQuad(e->exploding_point.x,e->exploding_point.y,10,10,flip_uvs);
        //m.uvs.clear();
        float w = 100, h = 100;
        Vector3 ep = e->exploding_point;
        Vector3 up = camera->up,
                side = (camera->center - camera->eye).cross(camera->up);
        up.normalize();
        side.normalize();
        //float uv_fpos = 1.0f/4.0f;
        Vector3 Hm = up*h*0.5,      //Medium height vector
                Wm = side*w*0.5;    //Medium width vector
        //---------CREATE QUAD---------
        m.vertices.push_back( ep+(Hm-Wm));      //Punto de colision + vector de traslación en el quad
        m.vertices.push_back( ep+(Hm + Wm));
        m.vertices.push_back( ep+(-Hm - Wm));

        m.vertices.push_back( ep+(Hm + Wm));
        m.vertices.push_back( ep+(-Hm + Wm));
        m.vertices.push_back( ep+(-Hm - Wm));

        float f = e->time / e->lifetime;
        unsigned frame = f * 25.0;

        float xt = frame % 5, yt = frame / 5;
        float step = 1.0f/5.0f;

        //texture coordinates
        m.uvs.push_back( Vector2(xt*step,yt*step + step));
        m.uvs.push_back( Vector2(xt*step +step,yt*step +step));
        m.uvs.push_back( Vector2(xt*step,yt*step));
        m.uvs.push_back( Vector2(xt*step +step,yt*step +step));
        m.uvs.push_back( Vector2(xt*step +step,yt*step));
        m.uvs.push_back( Vector2(xt*step,yt*step));
    }

    Texture* t = Texture::Load("explosion.tga",false);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);    //Pintar por los dos lados
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE); //El pixel ya pintado + la explosion con su alpha
    t->bind();
    m.render(GL_TRIANGLES);
    t->unbind();
    glEnable(GL_CULL_FACE);    //Pintar por los dos lados
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

}

void Explosion::updateAll(float elapsed_time) {
    if(!explosions.size())
        return;
    for(Explosion* e:explosions){
        e->time+= elapsed_time;
    }


    if((*explosions.begin())->time >= (*explosions.begin())->lifetime){     //Erase the first explosion if it has finished ( All ar ordered and in different frames )
        explosions.erase(explosions.begin());
    }
}