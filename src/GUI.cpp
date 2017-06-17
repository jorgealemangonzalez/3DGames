
#include "GUI.h"
#include "game.h"

GUI* GUI::gui = NULL;

GUI* GUI::getGUI() {
    if(gui == NULL){
        gui = new GUI;
    }
    return gui;
}

GUI::GUI() {
    Game* game = Game::instance;
    camera2d = new Camera();
    camera2d->setOrthographic(0, game->window_width, 0, game->window_height, 0, 1);
    debugPointsMesh = new Mesh();
    debugLinesMesh = new Mesh();
    guiPointsMesh = new Mesh();
    guiLinesMesh = new Mesh();

    std::string texture = "grid.tga";
    Texture::Load(texture,true);
    Mesh *plane = new Mesh();
    plane->createQuad(0,0,1000,1000);
    float tam = 100.f;
    plane->uvs[0] = Vector2(tam,tam);
    plane->uvs[1] = Vector2(0.0f,0.0f);
    plane->uvs[2] = Vector2(tam,0.0f);
    plane->uvs[3] = Vector2(0.0f,tam);
    plane->uvs[4] = Vector2(0.0f,0.0f);
    plane->uvs[5] = Vector2(tam,tam);
    plane->info.radius = tam*2;
    Mesh::s_Meshes["_grid"] = plane;
    grid = new EntityCollider();
    grid->setMesh("_grid");
    grid->setTexture(texture);
    show_grid = false;
    gui = this;
}

void GUI::setGridCenter(Vector3 center){

    grid->model.setTranslation(center);
    Vector3 axis(1.0,0,0);
    grid->model.rotateLocal(DEG2RAD*90,axis);
}

void GUI::render() {
    if(show_grid){
        //glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid->render(Game::instance->camera);
        glDisable(GL_BLEND);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
    Game* game = Game::instance;
    if(show_grid && game->mouse_when_press.x != -1){    //TODO cuando levanta el mouse crear plano perpendicular y calcular la interseccion con la posicion del mouse para mandar la uni alli
        guiLinesMesh->vertices.push_back(Vector3(game->mouse_when_press.x,game->mouse_when_press.y,0.99));
        guiLinesMesh->vertices.push_back(Vector3(game->mouse_when_press.x,game->window_height-game->mouse_position.y,0.99));
        guiLinesMesh->colors.push_back(Vector4(1,1,1,0));
        guiLinesMesh->colors.push_back(Vector4(1,1,1,0));
    }

    camera2d->set();
    glDisable(GL_DEPTH_TEST);
    if(debugMode){
        if(debugPointsMesh->vertices.size())
            debugPointsMesh->render(GL_POINTS);
        if(debugLinesMesh->vertices.size())
            debugLinesMesh->render(GL_LINES);
    }
    if(guiPointsMesh->vertices.size())
        guiPointsMesh->render(GL_POINTS);
    if(guiLinesMesh->vertices.size())
        guiLinesMesh->render(GL_LINES);
    glEnable(GL_DEPTH_TEST);
    Game::instance->camera->set();

    //debugPointsMesh->clear();
    //debugLinesMesh->clear();
    guiPointsMesh->clear();
    guiLinesMesh->clear();
}

Vector4 GUI::getColor(std::string team, bool selected) {
    int t = 0;
    if(team.compare("t1")==0) t = 1;
    else if(team.compare("t2")==0) t = 2;

    switch(t){
        case 1:
            if(selected)
                return Vector4(0.7, 0.9, 0.7, 1);
            return Vector4(0.4, 0.9, 0.4, 1);
        case 2:
            return Vector4(0.9, 0.4, 0.4, 1);
        default:
            return Vector4(0.4, 0.4, 0.7, 1);
    }
}

void GUI::addPoint(Vector3 pos1, bool debug, Vector4 color, bool projected) {
    if(!projected){
        Game* game = Game::instance;
        Camera* camera = game->camera;
        pos1 = camera->project(pos1, game->window_width, game->window_height);
    }
    if(debug){
        debugPointsMesh->vertices.push_back(pos1);
        debugPointsMesh->colors.push_back(color);
    }else{
        guiPointsMesh->vertices.push_back(pos1);
        guiPointsMesh->colors.push_back(color);
    }
}

void GUI::addLine(Vector3 pos1, Vector3 pos2, bool debug, Vector4 color, bool projected) {
    if(!projected){
        Game* game = Game::instance;
        Camera* camera = game->camera;
        pos1 = camera->project(pos1, game->window_width, game->window_height);
        pos2 = camera->project(pos2, game->window_width, game->window_height);
    }
    if(debug){
        debugLinesMesh->vertices.push_back(pos1);
        debugLinesMesh->vertices.push_back(pos2);
        debugLinesMesh->colors.push_back(color);
        debugLinesMesh->colors.push_back(color);
    }else{
        guiLinesMesh->vertices.push_back(pos1);
        guiLinesMesh->vertices.push_back(pos2);
        guiLinesMesh->colors.push_back(color);
        guiLinesMesh->colors.push_back(color);
    }
}

void GUI::showHideGrid(){
    show_grid = !show_grid;
}
