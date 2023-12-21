#include "hole_filling.h"
#include <QString>
#include <QDebug>

using namespace std ;
using namespace MeshReconstruction;

/* ************** Implicit RBF ************** */

Implicit_RBF::Implicit_RBF(vector<float> alpha, vector<float> beta, vector<MyMesh::Point> center) : _alpha(alpha), _beta(beta), _center(center)
{
    _n = _alpha.size() ;
    _d = _beta.size() ;
    if (_center.size() != _n)
        throw runtime_error("Inconsistent size of alpha and centers in Implicit_RBF constructor.");
}

// Computation of the value of the implicit surface at point X
float Implicit_RBF::val(MyMesh::Point X) const
{
    float res = 0 ;
    // Computation of the sum of RBF at centers
    for(int i=0; i<_n; i++)
    {
        res += _alpha.at(i) * myphi((X-_center.at(i)).norm()) ;
    }
    // Computation of the polynomial part
    for(int j=0; j<_d; j++)
    {
        res += _beta.at(j) * myp(j, X) ;
    }
    return res ;
}

/* ************** Hole Filling ************** */

Hole_Filling::Hole_Filling(MyMesh &mesh) : _mesh(mesh)
{
    _mesh.add_property(_vprop,  "vprop_flag");
    cout << "Starting hole filling ..." << endl ;
}

// ***** Computation of boundary and its neighborhood

MyMesh::HalfedgeHandle Hole_Filling::find_boundary_edge()
{
    MyMesh::HalfedgeIter he_it = _mesh.halfedges_begin();
    while ( (he_it != _mesh.halfedges_end()) && (!_mesh.is_boundary(*he_it)))
    {
        ++he_it ;
    }
    if (he_it != _mesh.halfedges_end())
        return *he_it ;
    else
        throw std::runtime_error("Boundary HE does not exist") ;
}

vector<MyMesh::VertexHandle> Hole_Filling::find_boundary(MyMesh::HalfedgeHandle heh)
{
    MyMesh::HalfedgeHandle heh_ini = heh ;
    vector<MyMesh::VertexHandle> boundary ;

    // Follow (and memorize) boundary edges
    do
    {
        boundary.push_back(_mesh.to_vertex_handle(heh));
        heh = _mesh.next_halfedge_handle(heh);
    } while (heh != heh_ini) ;
    return boundary ;
}

void Hole_Filling::init_mark_boundary(const vector<MyMesh::VertexHandle> & bnd)
{
    for (MyMesh::VertexIter v_it = _mesh.vertices_begin() ; v_it != _mesh.vertices_end(); ++v_it)
        _mesh.property(_vprop, *v_it) = false ;
    for (int i=0; i<bnd.size(); ++i)
        _mesh.property(_vprop, bnd.at(i)) = true ;
}

vector<MyMesh::VertexHandle> Hole_Filling::next_neighbors(const vector<MyMesh::VertexHandle> & bnd)
{
    // Visit bnd vertices to find and mark next circle
    vector<MyMesh::VertexHandle> next_bnd ;
    for (int i=0; i<bnd.size(); i++)
    {
        for (MyMesh::VertexVertexIter vv_it = _mesh.vv_iter(bnd.at(i));vv_it.is_valid();++vv_it)
        {
            if (_mesh.property(_vprop, *vv_it) == false) // new vertex
            {
                _mesh.property(_vprop, *vv_it) = true ;
                next_bnd.push_back(*vv_it);
            }
        }
    }
    return next_bnd ;
}

// ***** Computation of RBF

pair<pair<Eigen::MatrixXd &, Eigen::VectorXd &>, vector<MyMesh::Point> &> Hole_Filling::compute_approx_mat(vector<MyMesh::VertexHandle> vlist)
{
    const int n(vlist.size()), d(10) ;
    Eigen::MatrixXd & A = *(new Eigen::MatrixXd(3*n+d,3*n+d)) ;
    Eigen::MatrixXd Phi(3*n,3*n);
    Eigen::MatrixXd P(3*n,d);
    Eigen::VectorXd & B = *(new Eigen::VectorXd(3*n+d));

    vector<MyMesh::Point> & pts_list = *(new vector<MyMesh::Point>);
    //Append vertices to pts_list
    for (int i=0; i<n; i++)
    {
        pts_list.push_back(_mesh.point(vlist.at(i))) ;
    }
    //Append vertices+normals to pts_list
    for (int i=0; i<n; i++)
    {
        pts_list.push_back(_mesh.point(vlist.at(i)) + _mesh.normal(vlist.at(i))) ;
    }
    //Append vertices-normals to pts_list
    for (int i=0; i<n; i++)
    {
        pts_list.push_back(_mesh.point(vlist.at(i)) - _mesh.normal(vlist.at(i))) ;
    }

    int nn = pts_list.size() ;
    // Compute corresponding B vector (0 / 1 / -1 )
    B << Eigen::VectorXd::Zero(n), Eigen::VectorXd::Ones(n), -Eigen::VectorXd::Ones(n), Eigen::VectorXd::Zero(d) ;

    // Fill Phi matrix
    for(int i = 1; i < 3*n; i++){
        for(int j = 0; j< i; j++){
            Phi(i, j) = 0;
        }
    }

    for(int i = 0; i < 3*n; i++){
        for(int j = i; j<3*n; j++){
            Vec3f ci_cj = pts_list.at(i) - pts_list.at(j);
            double norm = ci_cj.norm();
            Phi(i,j) = myphi(norm);
        }
    }

    // Fill P matrix
    for(int i = 0; i< 3*n; i++){
        for(int j=0; j<d; j++){
            P(i,j) = myp(j, pts_list.at(i));
        }
    }
    //TODO
    // Set final A matrix
    /* A = Phi | P
     *      P' | 0    */
    // Rajout de Phi dans A
    for(int i = 0; i<3*n; i++){
        for(int j=0; j<3*n; j++){
            A(i,j) = Phi(i,j);
        }
    }

    // Rajout de P dans
    for(int i = 0; i<3*n; i++){
        for(int j= 3*n; j<3*n + d; j++){
            A(i,j) = P(i,j-(3*n));
        }
    }

    //Rajout de la transposé de P
    Eigen::MatrixXd P_t = P.transpose();
    for(int i = 3*n; i<3*n + d; i++){
        for(int j= 0; j<3*n; j++){
            A(i,j) = P_t(i-(3*n),j);
        }
    }

    // 0 pour le reste
    for(int i = 3*n; i<3*n +d; i++){
        for(int j= 3*n; j<3*n + d; j++){
            A(i,j) = 0;
        }
    }

    cout << "size of pts_list : " << nn << endl ;
    cout << "End computation of matrices" << endl ;

    return {{A,B},pts_list} ;
}

pair<vector<float>&, vector<float>&> Hole_Filling::solve_approx(const pair<Eigen::MatrixXd &, Eigen::VectorXd &> &p, int n, int d)
{
    Eigen::MatrixXd & A = p.first ;
    Eigen::VectorXd & B = p.second ;

    Eigen::VectorXd res = A.householderQr().solve(B) ;

    cout << "End of solver" << endl ;
    cout << "res : " << res.head(10) << endl ;
    vector<float> & alpha = *(new vector<float>);
    vector<float> & beta = *(new vector<float>);

    if (res.size() != (n+d))
    {
        cout << "taille du res : " << res.size() << endl ;
        throw std::runtime_error("Error in solve_approx") ;
    }
    for (int i=0; i<n; i++)
    {
        alpha.push_back(res(i)) ;
    }

    for (int j=0; j<d; j++)
    {
        beta.push_back(res(n+j)) ;
    }

    return {alpha, beta} ;
}

// ***** Hole filling
void concat_vectors(vector<VertexHandle> boundary, vector<VertexHandle> n_neighbors){
    for(VertexHandle v : n_neighbors){
        boundary.push_back(v);
    }
}

void Hole_Filling::set_color_boundary(){
    for(MyMesh::VertexIter v_it = _mesh.vertices_begin(); v_it != _mesh.vertices_end(); v_it++){
        if (_mesh.property(_vprop, *v_it)){
            _mesh.set_color(*v_it, MyMesh::Color(180, 0, 0));
        }
        else _mesh.set_color(*v_it, MyMesh::Color(150, 150, 150));
    }
}

void Hole_Filling::fill_hole(string out)
{
    // TODO !!!
    cout << "Au travail !" << endl ;
    //detections d'un trou
    HalfedgeHandle eh_boundary = find_boundary_edge();
    vector<VertexHandle> boundary = find_boundary(eh_boundary);
    init_mark_boundary(boundary);
    vector<VertexHandle> _1_neighbors = next_neighbors(boundary);
    concat_vectors(boundary, _1_neighbors);
    vector<VertexHandle> _2_neighbors = next_neighbors(_1_neighbors);
    concat_vectors(boundary, _2_neighbors);
    set_color_boundary();

    //On recupere la matrice
    pair<pair<Eigen::MatrixXd &, Eigen::VectorXd &>, vector<MyMesh::Point> &> approx = compute_approx_mat(boundary);
    pair<vector<float>&, vector<float>&> alphas_betas = solve_approx(approx.first, 3*boundary.size(), 10);
    Rect3 domain = estimate_BB(boundary);
    Implicit_RBF rbf = Implicit_RBF(alphas_betas.first, alphas_betas.second, approx.second);

    poly_n_out(rbf, domain, out);
}

// ***** IO

void Hole_Filling::colorize_prop()
{
    for (MyMesh::VertexIter v_it = _mesh.vertices_begin(); v_it != _mesh.vertices_end() ; ++v_it)
    {
        if(_mesh.property(_vprop, *v_it) == true)
            _mesh.set_color(*v_it, MyMesh::Color(255, 0, 0)) ;
        else
            _mesh.set_color(*v_it, MyMesh::Color(200, 200, 200)) ;
    }
}

void Hole_Filling::colorize_verts(const vector<MyMesh::VertexHandle> &vlist)
{
    for (MyMesh::VertexIter v_it = _mesh.vertices_begin(); v_it != _mesh.vertices_end() ; ++v_it)
    {
        _mesh.set_color(*v_it, MyMesh::Color(200, 200, 200)) ;
    }

    for(int i=0; i<vlist.size(); i++)
    {
        _mesh.set_color(vlist.at(i), MyMesh::Color(255, 0, 0)) ;
    }
}

Rect3 Hole_Filling::estimate_BB(const vector<MyMesh::VertexHandle> &vlist)
{
    MyMesh::Point minp = _mesh.point(vlist.at(0)), maxp=minp ;
    for (int i=0; i<vlist.size(); i++)
    {
        minp = min(minp, _mesh.point(vlist.at(i))) ;
        maxp = max(maxp, _mesh.point(vlist.at(i))) ;
    }
    MyMesh::Point sizep(maxp-minp), centerp = (minp+maxp)/2 ;
    minp = centerp - _scale*sizep/2 ;
    sizep *= _scale ;

    Rect3 domain ;
    domain.min = {minp[0], minp[1], minp[2]} ;
    domain.size = {sizep[0], sizep[1], sizep[2]} ;
    return domain ;
}

void Hole_Filling::poly_n_out(const Implicit_RBF &implicit, Rect3 domain, string filename)
{
    auto implicitEq = [&implicit](Vec3 const& pos)
    {
        MyMesh::Point X(pos.x, pos.y, pos.z) ;
        return implicit.val(X) ;
    };


    Vec3 cubeSize(domain.size*(_discr)) ;
    cout << "mind " << domain.min << endl ;
    cout << "sized" << domain.size << endl ;
    cout << "cubesize "<< cubeSize << endl ;

    auto mesh = MarchCube(implicitEq, domain, cubeSize);
    WriteObjFile(mesh, filename);
}
