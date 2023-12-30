#include "holefill.h"

holeFill::holeFill() {}

double holeFill::triangleArea(MyMesh* _mesh, const VertexHandle& v1, const VertexHandle& v2, const VertexHandle& v3) {
    const auto& p1 = _mesh->point(v1);
    const auto& p2 = _mesh->point(v2);
    const auto& p3 = _mesh->point(v3);

    OpenMesh::Vec3f vec1 = p2 - p1;
    OpenMesh::Vec3f vec2 = p3 - p1;

    return 0.5 * (vec1 % vec2).length();
}


void holeFill::triangularHole(MyMesh* mesh, std::vector<VertexHandle>& holeVertices) {
    if (holeVertices.size() < 3) {
        return;
    }

    size_t mid_index = holeVertices.size() / 2;
    VertexHandle vk = holeVertices[mid_index];

    VertexHandle v0 = holeVertices.front();
    VertexHandle vn = holeVertices.back();

    MyMesh::Point p0 = mesh->point(v0);
    MyMesh::Point pk = mesh->point(vk);
    MyMesh::Point pn = mesh->point(vn);

    OpenMesh::Vec3uc red(255, 0, 0);
    createTriangle(mesh, p0, pk, pn, red);

    if (holeVertices.size() > 3) {
        std::vector<VertexHandle> newHole1(holeVertices.begin(), holeVertices.begin() + mid_index + 1);
        std::vector<VertexHandle> newHole2(holeVertices.begin() + mid_index, holeVertices.end());

        triangularHole(mesh, newHole1);
        triangularHole(mesh, newHole2);
    }
}

void holeFill::createTriangle(MyMesh* mesh, MyMesh::Point& p1, MyMesh::Point& p2, MyMesh::Point& p3,  OpenMesh::Vec3uc& color) {
    if (!mesh) {
        return;
    }

    VertexHandle v1 = mesh->add_vertex(p1);
    VertexHandle v2 = mesh->add_vertex(p2);
    VertexHandle v3 = mesh->add_vertex(p3);

    mesh->set_color(v1, color);
    mesh->set_color(v2, color);
    mesh->set_color(v3, color);

    mesh->add_face(v1, v2, v3);

}
