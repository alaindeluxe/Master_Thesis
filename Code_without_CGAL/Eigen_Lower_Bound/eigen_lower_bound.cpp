#include <iostream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>
#include <algorithm>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

using namespace std;
using namespace Eigen;

std::vector<int> edge_i, edge_j;
std::vector<long> x,y;

//helper function taken from https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
bool ccw(int A, int B, int C){
    //return (C.y-A.y) * (B.x-A.x) > (B.y-A.y) * (C.x-A.x)
    return ((y[C]-y[A])*(x[B]-x[A])) > ((y[B]-y[A])*(x[C]-x[A]));
}

// Return true if line segments AB and CD intersect
bool doIntersect(int A, int B, int C, int D){
    if ((B==C) || (B==D) || (A==C) || (A==D)) {return false;}
    else {return ((ccw(A,C,D)!=ccw(B,C,D)) && (ccw(A,B,C)!=ccw(A,B,D)));}
}

bool myComparison(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second<b.second;
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/rvisp3499.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();

    x.clear(); y.clear();
    for (int i=0; i<n; i++){
        x.push_back(graphJson["x"][i].asInt64());
        y.push_back(graphJson["y"][i].asInt64());
    }

    std::vector<std::pair<int, int>> edges; //edge id's start at 0
    edges.clear();edge_i.clear();edge_j.clear();

    for (int i=0; i<m; i++){
        edge_i.push_back(graphJson["edge_i"][i].asInt());
        edge_j.push_back(graphJson["edge_j"][i].asInt());
        edges.push_back(std::make_pair(edge_i[i], edge_j[i]));
    }

    //perform algorithm
    //we want to find a large clique in the intersection graph
    //we do this by finding a large independent set in the complement of the intersection graph
    //compute complement of intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_matrix;
    adjacency_matrix.clear();
    for (int l=0; l<m; l++){
        adjacency_matrix.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(0), otherwise push_back(1)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {adjacency_matrix[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_matrix[l].push_back(1);
            } else {adjacency_matrix[l].push_back(0);}
        }
    }

    //determine degrees in complement of intersection graph
    //std::vector<std::pair<int,int>> degree_complement_intersection_graph; degree_complement_intersection_graph.clear();

    //define adjacency_matrix
    MatrixXd adj_matrix(m,m);
    for (int i=0; i<m; i++){
        for (int j=0; j<m; j++){
            //cout << i << " " << j << "\n";
            adj_matrix(i,j)=adjacency_matrix[i][j];
        }
    }

    //compute eigenvalues of adjacency matrix
    SelfAdjointEigenSolver<MatrixXd> es;
    cout << "Computing eigenvalues" << "\n";
    es.compute(adj_matrix, EigenvaluesOnly);
    VectorXd eig_vec=es.eigenvalues();
    //cout << es.eigenvalues() << "\n";

    double s_plus=0;
    double s_minus=0;

    for (int i=0; i<m; i++){
        if (eig_vec(i)>0) {
            double t=eig_vec(i)*eig_vec(i);
            s_plus=s_plus+t;
        } else if (eig_vec(i)<0) {
            double t=eig_vec(i)*eig_vec(i);
            s_minus=s_minus+t;
        }
    }

    cout << "Lower bound for the chromatic number of current graph: " << 1+(double) s_plus/(double) s_minus << "\n";

}