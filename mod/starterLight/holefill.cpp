#include "holefill.h"
#include <deque>
holeFill::holeFill() {}

double holeFill::triangleArea(MyMesh* _mesh, const VertexHandle& v1, const VertexHandle& v2, const VertexHandle& v3) {
    const auto& p1 = _mesh->point(v1);
    const auto& p2 = _mesh->point(v2);
    const auto& p3 = _mesh->point(v3);

    OpenMesh::Vec3f vec1 = p2 - p1;
    OpenMesh::Vec3f vec2 = p3 - p1;

    return 0.5 * (vec1 % vec2).length();
}

std::tuple<int, int> holeFill::cycle3_origins(MyMesh &mesh, FaceHandle fh, const std::vector<VertexHandle> &boundary_loop) {
    std::vector<int> face_vertex_indices;
    for (auto fv_it = mesh.cfv_begin(fh); fv_it != mesh.cfv_end(fh); ++fv_it) {
        int index = std::distance(boundary_loop.begin(), std::find(boundary_loop.begin(), boundary_loop.end(), *fv_it));
        if (index == boundary_loop.size()) {
            // Vertex not in boundary loop, push -1
            face_vertex_indices.push_back(-1);
        } else {
            face_vertex_indices.push_back(index);
        }
    }

    if (face_vertex_indices.size() != 3) {
        throw std::runtime_error("Input face is not triangular.");
    }

    std::sort(face_vertex_indices.begin(), face_vertex_indices.end());
    int i = face_vertex_indices[0], j = face_vertex_indices[1], k = face_vertex_indices[2];
    int n = boundary_loop.size();

    if (i == -1) {
        if (j == 0 && k == n - 1) {
            return std::make_tuple(n - 1, -1);
        }
        if (j + 1 == k) {
            return std::make_tuple(j, -1);
        }
        return std::make_tuple(-1, -1); // Internal face
    }
    if (i == 0 && k == n - 1) {
        if (j == 1) {
            return std::make_tuple(n - 1, 0);
        }
        if (j == n - 2) {
            return std::make_tuple(n - 2, n - 1);
        }
        throw std::runtime_error("Error in boundary loop.");
    }
    return std::make_tuple(i, j);
}

std::vector<FaceHandle> holeFill::fillHole(MyMesh* mesh, const std::vector<VertexHandle>& boundary_loop) {
    std::vector<FaceHandle> new_faces;
    if (!mesh || boundary_loop.size() < 3) return new_faces;

    // 确保所有边界顶点都是有效的
    for (auto & vh : boundary_loop) {
        if (!mesh->is_valid_handle(vh)) {
            qDebug("Invalid vertex handle in boundary loop");
            return new_faces;
        }
    }
    int n = boundary_loop.size();
    std::vector<std::vector<double>> areas(n, std::vector<double>(n, 0.0));
    std::vector<std::vector<int>> lambdas(n, std::vector<int>(n, -1));

    // 初始化直接相连的顶点对形成的三角形的面积
    for (int i = 0; i < n - 2; ++i) {
        areas[i][i + 2] = triangleArea(mesh, boundary_loop[i], boundary_loop[i + 1], boundary_loop[i + 2]);
    }
    qDebug("here0");
    // 使用动态规划填补孔洞
    for (int gap = 2; gap < n; ++gap) {
        for (int i = 0; i < n - gap; ++i) {
            int j = i + gap;
            double min_area = std::numeric_limits<double>::max();
            int optimal_m = -1;

            for (int m = i + 1; m < j; ++m) {
                double area = areas[i][m] + areas[m][j] + triangleArea(mesh, boundary_loop[i], boundary_loop[m], boundary_loop[j]);
                if (area < min_area) {
                    min_area = area;
                    optimal_m = m;
                }
            }

            areas[i][j] = min_area;
            lambdas[i][j] = optimal_m;
        }
    }
    // 重建填补孔洞的三角剖分
    std::deque<std::pair<int, int>> sections = {{0, n - 1}};
    while (!sections.empty()) {
        auto [d, b] = sections.front();
        sections.pop_front();
        if (b - d < 2) continue;

        int m = lambdas[d][b];
        if (m - d > 1) {
            FaceHandle new_face = mesh->add_face(boundary_loop[d], boundary_loop[m], boundary_loop[b]);
            new_faces.push_back(new_face);
            if (!mesh->is_valid_handle(new_face)) {
                qDebug() << "Failed to add new face using vertices" << d << m << b;
                continue;  // 或者根据你的需要选择合适的错误处理方式
            }
        }

        if (m - d > 1) {
            sections.push_front({d, m});
        }
        if (b - m > 1) {
            sections.push_front({m, b});
        }
    }
    return new_faces;
}

// OpenMesh::Vec3f holeFill::triangleNormal(MyMesh* _mesh, const Triangle& t) {
//     const auto& p1 = _mesh->point(MyMesh::VertexHandle(t.v1));
//     const auto& p2 = _mesh->point(MyMesh::VertexHandle(t.v2));
//     const auto& p3 = _mesh->point(MyMesh::VertexHandle(t.v3));

//     OpenMesh::Vec3f v1 = p2 - p1;
//     OpenMesh::Vec3f v2 = p3 - p1;

//     OpenMesh::Vec3f normal = v1 % v2;
//     normal.normalize();
//     return normal;
// }
