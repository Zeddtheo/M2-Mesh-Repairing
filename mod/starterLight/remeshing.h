#ifndef REMESHING_H
#define REMESHING_H

#include "mainwindow.h"
class Remeshing
{
public:
    Remeshing();

    static void split_long_edges(MyMesh *_mesh, double high);
    static void collapse_short_edges(MyMesh *_mesh, double low, double high);
    static void equalize_valences(MyMesh *_mesh);
    static void tangential_relaxation(MyMesh *_mesh);
    static void project_to_surface();
    static void remesh(MyMesh *_mesh, double target_edge_length);

    static int calculateValenceExcess(MyMesh *_mesh, MyMesh::EdgeHandle e_handle);
};

#endif // REMESHING_H
