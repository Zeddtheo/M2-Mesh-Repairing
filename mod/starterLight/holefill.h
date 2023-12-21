#ifndef HOLEFILL_H
#define HOLEFILL_H
#include <Eigen/Dense>
#include "mainwindow.h"
class holeFill
{
public:
    holeFill();
    static double triangleArea(MyMesh* _mesh, const VertexHandle& v1, const VertexHandle& v2, const VertexHandle& v3);
    //OpenMesh::Vec3f triangleNormal(MyMesh* _mesh, const Triangle& t);
    std::tuple<int, int> cycle3_origins(MyMesh &mesh, FaceHandle fh, const std::vector<VertexHandle> &boundary_loop);
    static std::vector<FaceHandle> fillHole(MyMesh* mesh, const std::vector<VertexHandle>& boundary_loop);
};

#endif // HOLEFILL_H
