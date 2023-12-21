#include "remeshing.h"
#include "mainwindow.h"

Remeshing::Remeshing() {

}

void Remeshing::split_long_edges(MyMesh *_mesh, double high) {
    std::unordered_set<MyMesh::EdgeHandle> dynamic_edges;
    for (auto e_it = _mesh->edges_begin(); e_it != _mesh->edges_end(); ++e_it)
        dynamic_edges.insert(*e_it);

    while (!dynamic_edges.empty()) {
        auto it = dynamic_edges.begin();
        auto e_handle = *it;
        dynamic_edges.erase(it);

        if (_mesh->status(e_handle).deleted())
            continue;

        if (_mesh->calc_edge_length(e_handle) > high) {
            MyMesh::HalfedgeHandle he_handle = _mesh->halfedge_handle(e_handle, 0);
            MyMesh::Point mid_point = (_mesh->point(_mesh->to_vertex_handle(he_handle)) +
                                       _mesh->point(_mesh->from_vertex_handle(he_handle))) / 2.0;
            auto new_vh = _mesh->split(e_handle, mid_point);

            if (new_vh.is_valid()) {
                for (auto voh_it = _mesh->voh_iter(new_vh); voh_it.is_valid(); ++voh_it) {
                    MyMesh::EdgeHandle new_eh = _mesh->edge_handle(*voh_it);
                    dynamic_edges.insert(new_eh);
                }
            }
        }
    }
}

void Remeshing::collapse_short_edges(MyMesh *_mesh, double low, double high) {
    std::vector<MyMesh::EdgeHandle> short_edges;
    for (auto e_it = _mesh->edges_begin(); e_it != _mesh->edges_end(); ++e_it) {
        if (_mesh->calc_edge_length(*e_it) < low) {
            short_edges.push_back(*e_it);
        }
    }

    for (auto e_handle : short_edges) {
        if (!_mesh->status(e_handle).deleted()) {
            MyMesh::HalfedgeHandle he_handle = _mesh->halfedge_handle(e_handle, 0);

            auto v0 = _mesh->to_vertex_handle(he_handle);
            auto v1 = _mesh->from_vertex_handle(he_handle);

            bool can_collapse = true;
            for (auto vv_it = _mesh->vv_iter(v0); vv_it.is_valid(); ++vv_it) {
                if (*vv_it != v1 && (_mesh->point(*vv_it) - _mesh->point(v1)).length() > high) {
                    can_collapse = false;
                    break;
                }
            }

            if (can_collapse) {
                _mesh->collapse(he_handle);
            }
        }
    }
    _mesh->garbage_collection();
}

void Remeshing::equalize_valences(MyMesh *_mesh){
    std::vector<MyMesh::EdgeHandle> edges_to_flip;
    for (auto e_it = _mesh->edges_begin(); e_it != _mesh->edges_end(); ++e_it) {
        if (_mesh->is_boundary(*e_it))
            continue;

        int valenceExcess = calculateValenceExcess(_mesh, *e_it);
        _mesh->flip(*e_it);
        int flippedValenceExcess = calculateValenceExcess(_mesh, *e_it);
        _mesh->flip(*e_it);

        if (flippedValenceExcess < valenceExcess) {
            edges_to_flip.push_back(*e_it);
        }
    }

    for (auto e_handle : edges_to_flip) {
        if (!_mesh->status(e_handle).deleted()) {
            _mesh->flip(e_handle);
        }
    }
}

int Remeshing::calculateValenceExcess(MyMesh *_mesh, MyMesh::EdgeHandle e_handle) {
    auto he_handle = _mesh->halfedge_handle(e_handle, 0);
    auto opp_he_handle = _mesh->opposite_halfedge_handle(he_handle);

    auto v0 = _mesh->to_vertex_handle(he_handle);
    auto v1 = _mesh->to_vertex_handle(opp_he_handle);

    int targetValence = 6;
    int valenceExcess = std::abs(static_cast<int>(_mesh->valence(v0)) - targetValence) +
                        std::abs(static_cast<int>(_mesh->valence(v1)) - targetValence);

    for (auto vv_it = _mesh->vv_iter(v0); vv_it.is_valid(); ++vv_it) {
        if (*vv_it != v1) {
            valenceExcess += std::abs(static_cast<int>(_mesh->valence(*vv_it)) - targetValence);
        }
    }

    for (auto vv_it = _mesh->vv_iter(v1); vv_it.is_valid(); ++vv_it) {
        if (*vv_it != v0) {
            valenceExcess += std::abs(static_cast<int>(_mesh->valence(*vv_it)) - targetValence);
        }
    }

    return valenceExcess;
}

void Remeshing::tangential_relaxation(MyMesh *_mesh){
    std::vector<MyMesh::Point> new_positions(_mesh->n_vertices());

    for (auto v_it = _mesh->vertices_begin(); v_it != _mesh->vertices_end(); ++v_it) {
        MyMesh::Point centroid(0.0, 0.0, 0.0);
        double total_weight = 0.0;

        for (auto vv_it = _mesh->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) {
            double weight = 1.0;
            centroid += _mesh->point(*vv_it) * weight;
            total_weight += weight;
        }

        if (total_weight != 0.0) {
            centroid /= total_weight;
        }

        MyMesh::Point current_position = _mesh->point(*v_it);
        MyMesh::Normal normal = _mesh->normal(*v_it);

        double displacement = OpenMesh::dot(centroid - current_position, normal);
        MyMesh::Point new_position = current_position + displacement * normal;

        new_positions[v_it->idx()] = new_position;
    }

    int idx = 0;
    for (auto v_it = _mesh->vertices_begin(); v_it != _mesh->vertices_end(); ++v_it, ++idx) {
        _mesh->set_point(*v_it, new_positions[idx]);
    }
}

void Remeshing::remesh(MyMesh *_mesh, double target_edge_length){
    double low = 4.0/5.0 * target_edge_length;
    double high = 4.0/3.0 * target_edge_length;
    for(int i = 0; i < 10; i++){
        split_long_edges(_mesh, high);
        qDebug("split_long_edges");
        //collapse_short_edges(_mesh, low, high);
        qDebug("collapse_short_edges");
        //equalize_valences(_mesh);
        qDebug("equalize_valences");
        //tangential_relaxation(_mesh);
        qDebug("tangential_relaxation");
        //project_to_surface();
    }
}
