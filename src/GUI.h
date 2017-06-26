
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
    ~GUI();

    Camera* camera2d;
    Mesh* guiPointsMesh;
    Mesh* guiLinesMesh;
    Mesh* guiPlanesMesh;    //Used in selection at the moment
    EntityCollider* grid;
    bool show_grid;

    Mesh* debugPointsMesh;
    Mesh* debugLinesMesh;

    void render();

    static Vector4 getColor(std::string team = "neutral", bool selected = false);
    void addPoint(Vector3 pos1, Vector4 color = Vector4(1,1,1,1), bool projected=false, bool debug = false);
    void addLine(Vector3 pos1, Vector3 pos2, Vector4 color = Vector4(1,1,1,1), bool projected=false, bool debug = false);
    void addPlane(Vector2 pos1, Vector2 pos2, Vector4 color = Vector4(1,1,1,0.3));
    void addCenteredCircles(Vector3 center, float radius, Vector4 color = Vector4(0.6, 0, 1, 1));
    void showHideGrid();
    void setGrid(bool show = false, Vector3 center = Vector3(0,0,0));
};


#endif //TJE_FRAMEWORK_2017_GUI_H
