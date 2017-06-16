
#ifndef TJE_FRAMEWORK_2017_GUI_H
#define TJE_FRAMEWORK_2017_GUI_H

#include "camera.h"
#include "mesh.h"
class EntityCollider;

class GUI {
public:
    static GUI* gui;
    static GUI* getGUI();
    GUI();

    Camera* camera2d;
    Mesh* guiPointsMesh;
    Mesh* guiLinesMesh;
    EntityCollider* grid;
    bool show_grid;

    Mesh* debugPointsMesh;
    Mesh* debugLinesMesh;

    void render();

    static Vector4 getColor(std::string team = "neutral", bool selected = false);
    void addPoint(Vector3 pos1, bool debug = false, Vector4 color = Vector4(1,1,1,1), bool projected=false);
    void addLine(Vector3 pos1, Vector3 pos2, bool debug = false, Vector4 color = Vector4(1,1,1,1), bool projected=false);
    void showHideGrid();
    void setGridCenter(Vector3 center);
};


#endif //TJE_FRAMEWORK_2017_GUI_H
