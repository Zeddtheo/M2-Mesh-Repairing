#ifndef HOLEFILL_H
#define HOLEFILL_H
#include <Eigen/Dense>
#include "mainwindow.h"
class holeFill
{
public:
    holeFill();
    static void triangularHole(MyMesh* mesh, std::vector<VertexHandle>& holeVertices);
    double triangleArea(MyMesh* _mesh, const VertexHandle& v1, const VertexHandle& v2, const VertexHandle& v3);
    static void createTriangle(MyMesh* mesh, MyMesh::Point& p1, MyMesh::Point& p2, MyMesh::Point& p3,  OpenMesh::Vec3uc& color);
};

#endif // HOLEFILL_H
