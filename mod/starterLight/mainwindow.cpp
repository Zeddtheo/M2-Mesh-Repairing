#include "mainwindow.h"
#include "holefill.h"
#include "ui_mainwindow.h"
#include <random>
#include <stack>
#include <unordered_set>
#include <vector>
#include "remeshing.h"
//---------------------------------Basic Functions--------------------------------------//
std::multimap<size_t, std::vector<FaceHandle>> MainWindow::findAllConnectedComponents(MyMesh& mesh) {
    std::multimap<size_t, std::vector<FaceHandle>> components;
    std::unordered_set<FaceHandle> visited;

    for (auto f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
        FaceHandle fh = *f_it;
        if (visited.find(fh) == visited.end()) {
            std::vector<FaceHandle> component;
            std::stack<FaceHandle> stack;
            stack.push(fh);

            while (!stack.empty()) {
                FaceHandle current = stack.top();
                stack.pop();

                if (visited.find(current) == visited.end()) {
                    visited.insert(current);
                    component.push_back(current);

                    for (auto ff_it = mesh.ff_begin(current); ff_it != mesh.ff_end(current); ++ff_it) {
                        if (visited.find(*ff_it) == visited.end()) {
                            stack.push(*ff_it);
                        }
                    }
                }
            }

            components.insert(std::make_pair(component.size(), std::move(component)));
        }
    }

    return components;
}
//-------------------------------------Colors------------------------------------------//
// Random color
void MainWindow::colorOneComponent(std::vector<FaceHandle> component, MyMesh* mesh) {
    if (!mesh->has_face_colors()) {
        mesh->request_face_colors();
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    MyMesh::Color color(dis(gen), dis(gen), dis(gen));

    for (auto& fh : component) {
        mesh->set_color(fh, color);
    }
}
// Define color
void MainWindow::colorOneComponent(std::vector<FaceHandle> component, MyMesh* mesh, MyMesh::Color color) {
    if (!mesh->has_face_colors()) {
        mesh->request_face_colors();
    }

    for (auto& fh : component) {
        mesh->set_color(fh, color);
    }
}

void MainWindow::colorAllComponents(std::multimap<size_t, std::vector<FaceHandle>>& components, MyMesh* mesh) {
    for (auto& component_pair : components) {
        colorOneComponent(component_pair.second, mesh);
    }
}

void MainWindow::colorBorderLoops(std::vector<std::vector<EdgeHandle>> borderLoops, MyMesh* mesh, const MyMesh::Color& color, float pointSize) {
    if (!mesh->has_edge_colors()) {
        mesh->request_edge_colors();
    }

    if (!mesh->has_vertex_colors()) {
        mesh->request_vertex_colors();
    }

    for (const auto& loop : borderLoops) {
        for (const auto& eh : loop) {

            mesh->set_color(eh, color);

            HalfedgeHandle heh = mesh->halfedge_handle(eh, 0);
            VertexHandle vh1 = mesh->to_vertex_handle(heh);
            VertexHandle vh2 = mesh->from_vertex_handle(heh);

            mesh->set_color(vh1, color);
            mesh->set_color(vh2, color);

            mesh->data(vh1).thickness = pointSize;
            mesh->data(vh2).thickness = pointSize;
        }
    }
}

void MainWindow::colorComponentEdges(std::vector<FaceHandle> component, MyMesh* mesh, MyMesh::Color color) {
    if (!mesh->has_edge_colors()) {
        mesh->request_edge_colors();
    }

    for (auto& fh : component) {
        for (auto e_it = mesh->fe_begin(fh); e_it != mesh->fe_end(fh); ++e_it) {
            mesh->set_color(*e_it, color);
        }
    }
}

void MainWindow::colorAllComponentsEdges(std::multimap<size_t, std::vector<FaceHandle>>& components, MyMesh* mesh) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    MyMesh::Color color(dis(gen), dis(gen), dis(gen));

    for (auto& component_pair : components) {
        MyMesh::Color color(dis(gen), dis(gen), dis(gen));
        colorComponentEdges(component_pair.second, mesh, color);
    }
}
//-------------------------------------Logics------------------------------------------//
//Define how many main components we need
std::unordered_set<VertexHandle> MainWindow::findMainComponentsVertices(MyMesh* mesh, int nbMainComponents) {
    auto components = findAllConnectedComponents(*mesh);


    std::vector<std::pair<size_t, std::vector<FaceHandle>>> component_vector(components.begin(), components.end());
    std::sort(component_vector.begin(), component_vector.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    std::unordered_set<VertexHandle> mainComponentsVertices;
    for (int i = 0; i < nbMainComponents && i < component_vector.size(); ++i) {
        for (const auto& face : component_vector[i].second) {
            for (auto fv_it = mesh->fv_begin(face); fv_it.is_valid(); ++fv_it) {
                mainComponentsVertices.insert(*fv_it);
            }
        }
    }
    return mainComponentsVertices;
}
//Detect noise components
std::vector<std::vector<FaceHandle>> MainWindow::findNoise(MyMesh* mesh) {
    auto components = findAllConnectedComponents(*mesh);

    std::vector<std::pair<size_t, std::vector<FaceHandle>>> component_vector(components.begin(), components.end());
    std::sort(component_vector.begin(), component_vector.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    //Define 2 main components(for this rabbit)
    auto mainComponentsVertices = findMainComponentsVertices(mesh, 2);
    //Find noise components who have common points or close points with main components
    std::vector<std::vector<FaceHandle>> noiseComponents;
    std::unordered_set<int> noiseComponentIndices;
    for (const auto& component_pair : component_vector) {
        if (component_pair.second == component_vector[0].second ||
            (component_vector.size() > 1 && component_pair.second == component_vector[1].second)) {
            continue;
        }

        bool isNoiseComponent = false;
        for (const auto& face : component_pair.second) {
            for (auto fv_it = mesh->fv_begin(face); fv_it.is_valid(); ++fv_it) {
                MyMesh::Point p1 = mesh->point(*fv_it);

                for (const auto& mainVert : mainComponentsVertices) {
                    MyMesh::Point p2 = mesh->point(mainVert);
                    if (isClose(p1, p2)) {
                        isNoiseComponent = true;
                        break;
                    }
                }

                if (!isNoiseComponent) {
                    for (auto fv_it2 = mesh->fv_begin(face); fv_it2.is_valid(); ++fv_it2) {
                        if(fv_it2 != fv_it) {
                            MyMesh::Point p2 = mesh->point(*fv_it2);
                            if (isClose(p1, p2)) {
                                isNoiseComponent = true;
                                break;
                            }
                        }
                    }
                }

                if (isNoiseComponent) break;
            }
            if (isNoiseComponent) {
                noiseComponentIndices.insert(component_pair.second.front().idx());
                noiseComponents.push_back(component_pair.second);
                break;
            }
        }
    }

    //Detect if components who are not main nor noise have common points or close points with noisecomponents
    for (const auto& component_pair : component_vector) {
        if (noiseComponentIndices.find(component_pair.second.front().idx()) != noiseComponentIndices.end() ||
            component_pair.second == component_vector[0].second ||
            (component_vector.size() > 1 && component_pair.second == component_vector[1].second)) {
            continue;
        }

        bool isCloseToNoise = false;
        for (const auto& face : component_pair.second) {
            for (auto fv_it = mesh->fv_begin(face); fv_it.is_valid(); ++fv_it) {
                MyMesh::Point p1 = mesh->point(*fv_it);

                for (auto idx : noiseComponentIndices) {
                    FaceHandle noiseFaceHandle(idx);
                    for (auto nf_it = mesh->fv_begin(noiseFaceHandle); nf_it.is_valid(); ++nf_it) {
                        MyMesh::Point p2 = mesh->point(*nf_it);
                        if (isClose(p1, p2) || *fv_it == *nf_it) {
                            isCloseToNoise = true;
                            break;
                        }
                    }
                    if (isCloseToNoise) break;
                }
                if (isCloseToNoise) break;
            }
            if (isCloseToNoise) {
                noiseComponents.push_back(component_pair.second);
                noiseComponentIndices.insert(component_pair.second.front().idx());
                break;
            }
        }
    }

    return noiseComponents;
}
//Detect close points, just for test
void MainWindow::markCloseVertices(MyMesh* _mesh) {
    for (MyMesh::VertexIter v_it1 = _mesh->vertices_begin(); v_it1 != _mesh->vertices_end(); ++v_it1) {
        for (MyMesh::VertexIter v_it2 = v_it1 + 1; v_it2 != _mesh->vertices_end(); ++v_it2) {
            MyMesh::Point p1 = _mesh->point(*v_it1);
            MyMesh::Point p2 = _mesh->point(*v_it2);

            if (isClose(p1,p2)) {
                _mesh->set_color(*v_it1, MyMesh::Color(255, 0, 0));
                _mesh->data(*v_it1).thickness = 10;
                _mesh->set_color(*v_it2, MyMesh::Color(255, 0, 0));
                _mesh->data(*v_it2).thickness = 10;
            }
        }
    }
}

bool MainWindow::isClose(MyMesh::Point p1, MyMesh::Point p2){
    double minDistance = 0.001;
    double distance = (p1 - p2).length();
    return distance < minDistance;
}

std::vector<std::vector<FaceHandle>> MainWindow::findFloaters(MyMesh* mesh) {
    auto components = findAllConnectedComponents(*mesh);
    auto noiseComponents = findNoise(mesh);
    auto mainComponentsVertices = findMainComponentsVertices(mesh, 2);

    std::unordered_set<FaceHandle> main_component_faces;
    for (const auto& vertex : mainComponentsVertices) {
        for (auto vf_it = mesh->vf_begin(vertex); vf_it.is_valid(); ++vf_it) {
            main_component_faces.insert(*vf_it);
        }
    }

    std::unordered_set<FaceHandle> noiseFaces;
    for (const auto& noiseComponent : noiseComponents) {
        for (const auto& face : noiseComponent) {
            noiseFaces.insert(face);
        }
    }

    std::vector<std::vector<FaceHandle>> floaters;
    for (const auto& component_pair : components) {
        if (main_component_faces.find(component_pair.second.front()) != main_component_faces.end() ||
            std::any_of(component_pair.second.begin(), component_pair.second.end(),
                        [&](const FaceHandle& face){ return noiseFaces.count(face) > 0; })) {
            continue;
        }

        bool is_floater = true;
        for (const auto& face : component_pair.second) {
            for (auto h_it = mesh->fh_begin(face); h_it != mesh->fh_end(face); ++h_it) {
                HalfedgeHandle he = *h_it;
                FaceHandle adjacent_face = mesh->opposite_face_handle(he);
                if (adjacent_face.is_valid() && main_component_faces.find(adjacent_face) != main_component_faces.end()) {
                    is_floater = false;
                    break;
                }
            }
            if (!is_floater) break;
        }
        if (is_floater) {
            floaters.push_back(component_pair.second);
        }
    }

    return floaters;
}

std::vector<std::vector<EdgeHandle>> MainWindow::findBorders(MyMesh* mesh) {
    auto mainComponentsVertices = findMainComponentsVertices(mesh, 2);

    std::unordered_set<FaceHandle> mainComponentFaces;
    for (const auto& vertex : mainComponentsVertices) {
        for (auto vf_it = mesh->vf_begin(vertex); vf_it.is_valid(); ++vf_it) {
            mainComponentFaces.insert(*vf_it);
        }
    }

    std::unordered_set<EdgeHandle> checkedEdges;
    std::vector<std::vector<EdgeHandle>> borderLoops;

    for (const auto& face : mainComponentFaces) {
        for (auto fe_it = mesh->fe_begin(face); fe_it.is_valid(); ++fe_it) {
            EdgeHandle eh = *fe_it;
            if (checkedEdges.find(eh) != checkedEdges.end()) {
                continue;
            }

            HalfedgeHandle heh = mesh->halfedge_handle(eh, 0);
            HalfedgeHandle opposite_heh = mesh->opposite_halfedge_handle(heh);

            if (mesh->is_boundary(heh) || mesh->is_boundary(opposite_heh)) {
                //qDebug("here!");
                std::vector<EdgeHandle> loop;
                EdgeHandle current_eh = eh;
                do {
                    loop.push_back(current_eh);
                    checkedEdges.insert(current_eh);

                    HalfedgeHandle current_heh = mesh->halfedge_handle(current_eh, 0);

                    HalfedgeHandle next_heh = mesh->next_halfedge_handle(current_heh);
                    while (!mesh->is_boundary(mesh->opposite_halfedge_handle(next_heh))) {
                        next_heh = mesh->next_halfedge_handle(mesh->opposite_halfedge_handle(next_heh));
                    }

                    current_eh = mesh->edge_handle(next_heh);
                } while (current_eh != eh && checkedEdges.find(current_eh) == checkedEdges.end());

                if (!loop.empty()) {
                    borderLoops.push_back(loop);
                }
            }
        }
    }

    std::sort(borderLoops.begin(), borderLoops.end(),
              [](const std::vector<EdgeHandle>& a, const std::vector<EdgeHandle>& b) {
                  return a.size() > b.size();
              });

    return borderLoops;
}

std::vector<std::vector<EdgeHandle>> MainWindow::findCracks(MyMesh* mesh) {

    auto borderLoops = findBorders(mesh);

    std::vector<std::vector<EdgeHandle>> cracks;


    for (int i = 0; i < 2 && i < borderLoops.size(); ++i) {
        cracks.push_back(borderLoops[i]);
    }

    return cracks;
}

std::vector<std::vector<EdgeHandle>> MainWindow::findHoles(MyMesh* mesh) {
    auto borderLoops = findBorders(mesh);
    auto cracks = findCracks(mesh);

    std::vector<std::vector<EdgeHandle>> holes;

    for (const auto& loop : borderLoops) {
        bool isCrack = false;
        for (const auto& crack : cracks) {
            if (loop.size() == crack.size() && std::equal(loop.begin(), loop.end(), crack.begin())) {
                isCrack = true;
                break;
            }
        }

        if (!isCrack) {
            holes.push_back(loop);
        }
    }

    return holes;
}

std::vector<VertexHandle> MainWindow::boundaryLoop(const std::vector<EdgeHandle>& edgeHandles, MyMesh* mesh) {
    std::vector<VertexHandle> loopVertices;
    if (edgeHandles.empty() || !mesh) return loopVertices;

    for (const auto& eh : edgeHandles) {
        auto heh = mesh->halfedge_handle(eh, 0);

        VertexHandle from_vh = mesh->from_vertex_handle(heh);
        VertexHandle to_vh = mesh->to_vertex_handle(heh);

        if (loopVertices.empty() || loopVertices.back() != from_vh) {
            loopVertices.push_back(from_vh);
        }
        loopVertices.push_back(to_vh);
    }

    if (!loopVertices.empty() && loopVertices.front() != loopVertices.back()) {
        loopVertices.push_back(loopVertices.front());
    }

    return loopVertices;
}
//-------------------------------------Show------------------------------------------//
void MainWindow::showComponents(MyMesh* _mesh){
    std::multimap<size_t, std::vector<FaceHandle>> componentFaces = findAllConnectedComponents(mesh);
    colorAllComponents(componentFaces, &mesh);
    //colorAllComponentsEdges(componentFaces, &mesh);
}

void MainWindow::showCracks(MyMesh* _mesh){
    //auto borderLoops = findBorders(_mesh);
    auto cracks = findCracks(_mesh);
    MyMesh::Color crackColor(255,0,0);
    colorBorderLoops(cracks, _mesh, crackColor,10);
}

void MainWindow::showHoles(MyMesh* _mesh) {
    //markCloseVertices(&mesh);
    auto holes = findHoles(_mesh);
    MyMesh::Color holeColor(0,255,0);
    colorBorderLoops(holes, _mesh, holeColor,10);
}

void MainWindow::showNoises(MyMesh* _mesh)
{
    auto noises = findNoise(_mesh);

    MyMesh::Color noiseColor(255, 255, 128);

    for (auto& n : noises) {
        colorOneComponent(n, &mesh, noiseColor);
    }
}

void MainWindow::showFloaters(MyMesh* mesh) {
    auto floaters = findFloaters(mesh);

    MyMesh::Color floatersColor(200, 100, 200);

    for (auto& floater : floaters) {
        colorOneComponent(floater, mesh, floatersColor);
    }
}

//--------------------------------------Fix-------------------------------------------//
void MainWindow::fixHoles(MyMesh* mesh) {
    if (!mesh) return;

    std::vector<std::vector<EdgeHandle>> holes = findHoles(mesh);
    qDebug("here5");
    for (const std::vector<EdgeHandle>& hole : holes) {
        std::vector<VertexHandle> boundary_vertices = boundaryLoop(hole, mesh);
        std::vector<FaceHandle> new_faces = holeFill::fillHole(mesh, boundary_vertices);
        for (const FaceHandle& new_face : new_faces) {
            mesh->set_color(new_face, MyMesh::Color(0, 0, 0));
        }
    qDebug("here6");
    }
}

void MainWindow::fixCracks(){

}

void MainWindow::fixNoises(MyMesh* mesh){
    auto noises = findNoise(mesh);

    for (const auto& noise : noises) {
        for (const auto& fh : noise) {
            if (!mesh->status(fh).deleted()) {
                mesh->delete_face(fh, true);
            }
        }
    }

    mesh->garbage_collection();
}

void MainWindow::fixFloaters(MyMesh* mesh) {
    auto floaters = findFloaters(mesh);

    for (const auto& floater : floaters) {
        for (const auto& fh : floater) {
            if (!mesh->status(fh).deleted()) {
                mesh->delete_face(fh, true);
            }
        }
    }

    mesh->garbage_collection();
}

void MainWindow::fixAll(){

}

/* **** début de la partie boutons et IHM **** */

// exemple pour charger un fichier .obj
void MainWindow::on_pushButton_chargement_clicked()
{
    // fenêtre de sélection des fichiers
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh"), "", tr("Mesh Files (*.obj)"));

    // chargement du fichier .obj dans la variable globale "mesh"
    OpenMesh::IO::read_mesh(mesh, fileName.toUtf8().constData());

    mesh.update_normals();

    // initialisation des couleurs et épaisseurs (sommets et arêtes) du mesh
    resetAllColorsAndThickness(&mesh);

    // on affiche le maillage
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Components_clicked(){
    showComponents(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Holes_clicked(){
    markCloseVertices(&mesh);
    //showHoles(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Cracks_clicked(){
    showCracks(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Noises_clicked(){
    showNoises(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Floaters_clicked(){
    showFloaters(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_FixHoles_clicked(){
    fixHoles(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_FixCracks_clicked(){

}

void MainWindow::on_pushButton_FixNoises_clicked(){
    fixNoises(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_FixFloaters_clicked(){
    fixFloaters(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_FixAll_clicked(){

}

void MainWindow::on_pushButton_Reset_clicked(){
    resetAllColorsAndThickness(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_Remesh_clicked()
{
    Remeshing::remesh(&mesh, 0.2);
    resetAllColorsAndThickness(&mesh);
    displayMesh(&mesh);
}
/* **** fin de la partie boutons et IHM **** */

/* **** fonctions supplémentaires **** */
// permet d'initialiser les couleurs et les épaisseurs des élements du maillage
void MainWindow::resetAllColorsAndThickness(MyMesh* _mesh)
{
    for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
    {
        _mesh->data(*curVert).thickness = 1;
        _mesh->set_color(*curVert, MyMesh::Color(0, 0, 0));
    }

    for (MyMesh::FaceIter curFace = _mesh->faces_begin(); curFace != _mesh->faces_end(); curFace++)
    {
        _mesh->set_color(*curFace, MyMesh::Color(150, 150, 150));
    }

    for (MyMesh::EdgeIter curEdge = _mesh->edges_begin(); curEdge != _mesh->edges_end(); curEdge++)
    {
        _mesh->data(*curEdge).thickness = 1;
        _mesh->set_color(*curEdge, MyMesh::Color(0, 0, 0));
    }
}
// charge un objet MyMesh dans l'environnement OpenGL
void MainWindow::displayMesh(MyMesh* _mesh, bool isTemperatureMap, float mapRange)
{
    GLuint* triIndiceArray = new GLuint[_mesh->n_faces() * 3];
    GLfloat* triCols = new GLfloat[_mesh->n_faces() * 3 * 3];
    GLfloat* triVerts = new GLfloat[_mesh->n_faces() * 3 * 3];

    int i = 0;

    if(isTemperatureMap)
    {
        QVector<float> values;

        if(mapRange == -1)
        {
            for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
                values.append(fabs(_mesh->data(*curVert).value));
            //qSort(values);
            std::sort(values.begin(), values.end());
            mapRange = values.at(values.size()*0.8);
            qDebug() << "mapRange" << mapRange;
        }

        float range = mapRange;
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;

        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }
    else
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }


    ui->displayWidget->loadMesh(triVerts, triCols, _mesh->n_faces() * 3 * 3, triIndiceArray, _mesh->n_faces() * 3);

    delete[] triIndiceArray;
    delete[] triCols;
    delete[] triVerts;

    GLuint* linesIndiceArray = new GLuint[_mesh->n_edges() * 2];
    GLfloat* linesCols = new GLfloat[_mesh->n_edges() * 2 * 3];
    GLfloat* linesVerts = new GLfloat[_mesh->n_edges() * 2 * 3];

    i = 0;
    QHash<float, QList<int> > edgesIDbyThickness;
    for (MyMesh::EdgeIter eit = _mesh->edges_begin(); eit != _mesh->edges_end(); ++eit)
    {
        float t = _mesh->data(*eit).thickness;
        if(t > 0)
        {
            if(!edgesIDbyThickness.contains(t))
                edgesIDbyThickness[t] = QList<int>();
            edgesIDbyThickness[t].append((*eit).idx());
        }
    }
    QHashIterator<float, QList<int> > it(edgesIDbyThickness);
    QList<QPair<float, int> > edgeSizes;
    while (it.hasNext())
    {
        it.next();

        for(int e = 0; e < it.value().size(); e++)
        {
            int eidx = it.value().at(e);

            MyMesh::VertexHandle vh1 = _mesh->to_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh1)[0];
            linesVerts[3*i+1] = _mesh->point(vh1)[1];
            linesVerts[3*i+2] = _mesh->point(vh1)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;

            MyMesh::VertexHandle vh2 = _mesh->from_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh2)[0];
            linesVerts[3*i+1] = _mesh->point(vh2)[1];
            linesVerts[3*i+2] = _mesh->point(vh2)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;
        }
        edgeSizes.append(qMakePair(it.key(), it.value().size()));
    }

    ui->displayWidget->loadLines(linesVerts, linesCols, i * 3, linesIndiceArray, i, edgeSizes);

    delete[] linesIndiceArray;
    delete[] linesCols;
    delete[] linesVerts;

    GLuint* pointsIndiceArray = new GLuint[_mesh->n_vertices()];
    GLfloat* pointsCols = new GLfloat[_mesh->n_vertices() * 3];
    GLfloat* pointsVerts = new GLfloat[_mesh->n_vertices() * 3];

    i = 0;
    QHash<float, QList<int> > vertsIDbyThickness;
    for (MyMesh::VertexIter vit = _mesh->vertices_begin(); vit != _mesh->vertices_end(); ++vit)
    {
        float t = _mesh->data(*vit).thickness;
        if(t > 0)
        {
            if(!vertsIDbyThickness.contains(t))
                vertsIDbyThickness[t] = QList<int>();
            vertsIDbyThickness[t].append((*vit).idx());
        }
    }
    QHashIterator<float, QList<int> > vitt(vertsIDbyThickness);
    QList<QPair<float, int> > vertsSizes;

    while (vitt.hasNext())
    {
        vitt.next();

        for(int v = 0; v < vitt.value().size(); v++)
        {
            int vidx = vitt.value().at(v);

            pointsVerts[3*i+0] = _mesh->point(_mesh->vertex_handle(vidx))[0];
            pointsVerts[3*i+1] = _mesh->point(_mesh->vertex_handle(vidx))[1];
            pointsVerts[3*i+2] = _mesh->point(_mesh->vertex_handle(vidx))[2];
            pointsCols[3*i+0] = _mesh->color(_mesh->vertex_handle(vidx))[0];
            pointsCols[3*i+1] = _mesh->color(_mesh->vertex_handle(vidx))[1];
            pointsCols[3*i+2] = _mesh->color(_mesh->vertex_handle(vidx))[2];
            pointsIndiceArray[i] = i;
            i++;
        }
        vertsSizes.append(qMakePair(vitt.key(), vitt.value().size()));
    }

    ui->displayWidget->loadPoints(pointsVerts, pointsCols, i * 3, pointsIndiceArray, i, vertsSizes);

    delete[] pointsIndiceArray;
    delete[] pointsCols;
    delete[] pointsVerts;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



