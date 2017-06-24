
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
    guiPlanesMesh = new Mesh();

    std::string texture = "grid.tga";
    Texture::Load(texture,true);
    Mesh *plane = new Mesh();
    plane->createQuad(0,0,10000,10000);
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

void GUI::render() {
    if(show_grid){
        //glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid->render(Game::instance->camera);
        glDisable(GL_BLEND);

        glEnable(GL_CULL_FACE);
        //glEnable(GL_DEPTH_TEST);
    }

    camera2d->set();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(debugMode){
        if(debugPointsMesh->vertices.size())
            debugPointsMesh->render(GL_POINTS);
        if(debugLinesMesh->vertices.size())
            debugLinesMesh->render(GL_LINES);
        debugPointsMesh->clear();
        debugLinesMesh->clear();
    }
    if(guiPointsMesh->vertices.size())
        guiPointsMesh->render(GL_POINTS);
    if(guiLinesMesh->vertices.size()) {
        guiLinesMesh->render(GL_LINES);
    }
    if(guiPlanesMesh->vertices.size()){
        glDisable(GL_CULL_FACE);
        guiPlanesMesh->render(GL_TRIANGLES);
        glEnable(GL_CULL_FACE);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    Game::instance->camera->set();

    guiPointsMesh->clear();
    guiLinesMesh->clear();
    guiPlanesMesh->clear();
}

Vector4 GUI::getColor(std::string team, bool selected) {
    int t = 0;
    if(team.compare(HUMAN_TEAM)==0) t = 1;
    else if(team.compare(ENEMY_TEAM)==0) t = 2;

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

void GUI::addPoint(Vector3 pos1, Vector4 color, bool projected, bool debug) {
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

void GUI::addLine(Vector3 pos1, Vector3 pos2, Vector4 color, bool projected, bool debug) {
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

void GUI::addPlane(Vector2 pos1, Vector2 pos2, Vector4 color) { //No debug, always projected
    float minX, maxX, minY, maxY;
    if(pos1.x < pos2.x){
        minX = pos1.x; maxX = pos2.x;
    }else{
        minX = pos2.x; maxX = pos1.x;
    }
    if(pos1.y < pos2.x){
        minY = pos1.y; maxY = pos2.y;
    }else{
        minY = pos2.y; maxY = pos1.y;
    }

    guiPlanesMesh->vertices.push_back(Vector3(maxX, maxY, 0));
    guiPlanesMesh->vertices.push_back(Vector3(minX, minY, 0));
    guiPlanesMesh->vertices.push_back(Vector3(maxX, minY, 0));
    guiPlanesMesh->vertices.push_back(Vector3(minX, maxY, 0));
    guiPlanesMesh->vertices.push_back(Vector3(minX, minY, 0));
    guiPlanesMesh->vertices.push_back(Vector3(maxX, maxY, 0));

    guiPlanesMesh->colors.push_back(color);
    guiPlanesMesh->colors.push_back(color);
    guiPlanesMesh->colors.push_back(color);
    guiPlanesMesh->colors.push_back(color);
    guiPlanesMesh->colors.push_back(color);
    guiPlanesMesh->colors.push_back(color);
}

void GUI::addCenteredCircles(Vector3 center, float radius, Vector4 color){

    Vector3 point;

    Game* game = Game::instance;
    Camera* camera = game->camera;
    Matrix44 m;
    m.setRotation(PI/2.0,Vector3(1,0,0));
    for(float rad = 0.1; rad < radius; rad+=radius/5) {
        float samples = int(2.0*PI*radius);//Depende de la longitud de la circuferencia

        float delta_angle = 2.0*PI/samples;
        float total_angle = delta_angle;
        for (int i = 0; i < samples; ++i) {
            point.x = cos(total_angle) * rad;
            point.y = sin(total_angle) * rad;
            point.z = 0;
            total_angle += delta_angle;

            point = camera->project(center + m.rotateVector(point), game->window_width, game->window_height);
            guiLinesMesh->vertices.push_back(point);
            guiLinesMesh->colors.push_back(color);
        }
    }

    if(guiLinesMesh->vertices.size() % 2 != 0)
        guiLinesMesh->vertices.pop_back(), guiLinesMesh->colors.pop_back();

}

void GUI::setGrid(bool show, Vector3 center) {
    show_grid = show;
    grid->model.setTranslation(center);
    Vector3 axis(1.0,0,0);
    grid->model.rotateLocal(DEG2RAD*90,axis);
}

void GUI::showHideGrid(){
    show_grid = !show_grid;
}
