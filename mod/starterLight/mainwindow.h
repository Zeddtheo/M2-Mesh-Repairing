#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>


namespace Ui {
class MainWindow;
}

using namespace OpenMesh;
using namespace OpenMesh::Attributes;

struct MyTraits : public OpenMesh::DefaultTraits
{
    // use vertex normals and vertex colors
    VertexAttributes( OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color | OpenMesh::Attributes::Status);
    // store the previous halfedge
    HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
    // use face normals face colors
    FaceAttributes( OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color | OpenMesh::Attributes::Status);
    EdgeAttributes( OpenMesh::Attributes::Color | OpenMesh::Attributes::Status );
    // vertex thickness
    VertexTraits{float thickness; float value; float taken;};
    // edge thickness
    EdgeTraits{float thickness;};
};
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyMesh;

using Component = std::vector<MyMesh::FaceHandle>;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void displayMesh(MyMesh *_mesh, bool isTemperatureMap = false, float mapRange = -1);
    void resetAllColorsAndThickness(MyMesh* _mesh);

    //std::vector<std::vector<FaceHandle>> findAllConnectedComponents(MyMesh& mesh);
    std::multimap<size_t, std::vector<FaceHandle>> findAllConnectedComponents(MyMesh& mesh);
    void colorOneComponent(std::vector<FaceHandle> component, MyMesh* mesh);
    void colorOneComponent(std::vector<FaceHandle> component, MyMesh* mesh, MyMesh::Color color);
    void colorAllComponents(std::multimap<size_t, std::vector<FaceHandle>>& components, MyMesh* mesh);
    void colorComponentEdges(std::vector<FaceHandle> component, MyMesh* mesh, MyMesh::Color color);
    void colorAllComponentsEdges(std::multimap<size_t, std::vector<FaceHandle>>& components, MyMesh* mesh);
    void showComponents(MyMesh* _mesh);

    void showHoles(MyMesh* _mesh);

    std::vector<std::vector<VertexHandle>> findGaps(MyMesh* _mesh);
    void showGaps(MyMesh* _mesh);

    std::vector<EdgeHandle> findEdgesAdjacentToThreeFaces(MyMesh* mesh);

    std::vector<std::vector<FaceHandle>> findNoise(MyMesh* mesh);
    void showNoises(MyMesh* _mesh);

    std::vector<std::vector<FaceHandle>> findFloaters(MyMesh* _mesh);
    void showFloaters(MyMesh* _mesh);

    void fixHoles();
    void fixCracks();
    void fixNoises(MyMesh* mesh);
    void fixFloaters(MyMesh* mesh);
    void fixAll();

    void markCloseVertices(MyMesh* _mesh);
    void mergeCloseVertices(MyMesh* _mesh);
    bool isClose(MyMesh::Point p1, MyMesh::Point p2);
private slots:
    void on_pushButton_chargement_clicked();

    void on_pushButton_Components_clicked();
    void on_pushButton_Holes_clicked();
    void on_pushButton_Cracks_clicked();
    void on_pushButton_Noises_clicked();
    void on_pushButton_Floaters_clicked();

    void on_pushButton_FixHoles_clicked();
    void on_pushButton_FixCracks_clicked();
    void on_pushButton_FixNoises_clicked();
    void on_pushButton_FixFloaters_clicked();
    void on_pushButton_FixAll_clicked();

    void on_pushButton_Reset_clicked();


private:

    MyMesh mesh;
    std::vector<float>boundaries_length;
    std::vector<std::vector<VertexHandle>> gaps;
    std::vector<std::vector<FaceHandle>> componentsFaces;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
