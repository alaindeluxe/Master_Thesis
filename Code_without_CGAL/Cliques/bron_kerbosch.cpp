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
#include <random>

using namespace std;

std::vector<int> edge_i, edge_j, p_vec, r_vec, x_vec, degrees;
std::vector<std::vector<int>> adjacency_list, adj_matrix;
std::vector<long> x,y;
int max_clique_size,n,m;

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
       return a.second>b.second;
}

bool myComparison3(const tuple<int,int,int> &a,const tuple<int,int,int> &b)
{
       return std::get<2>(a)>std::get<2>(b);
}

bool myComp_high_to_low(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second>b.second;
}

bool myComp_low_to_high(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second<b.second;
}

void BronKerbosch2(std::vector<int> r_vect, std::vector<int> p_vect, std::vector<int> x_vect){

    int p_size=0;
    int x_size=0;
    int r_size=0;
    for (int i=0; i<m; i++){
        p_size=p_size+p_vect[i];
        x_size=x_size+x_vect[i];
        r_size=r_size+r_vect[i];
    }

    //std::cout << r_size << " " << p_size << " " << x_size << "\n";

    if (p_size==0 && x_size==0){
        std::cout << "We found a clique of size " << r_size << "\n";
        if (r_size>max_clique_size) {max_clique_size=r_size;}
        std::cout << "Current largest clique: " << max_clique_size << "\n";
    }

    //find vertex in P union X such that P_minus_Nu is as small as possible

    std::vector<int> p_minus_nu_vect = p_vect;
    int max_d=0;
    int u=0;
    for (int i=0; i<m; i++){
        if (p_vect[i]==1 || x_vect[i]==1){
            if (degrees[i]>max_d){
                max_d=degrees[i];
                u=i;
            }
        }
    }

    for (int i=0; i<adjacency_list[u].size(); i++){
        p_minus_nu_vect[adjacency_list[u][i]]=0;
    }

    for (int i=0; i<m; i++){
        if (p_minus_nu_vect[i]==1){
            std::vector<int> r_vect2=r_vect;
            r_vect2[i]=1;

            std::vector<int> p_vect2; p_vect2.clear();
            for (int j=0; j<m; j++) {
                if (adj_matrix[i][j]==1 && p_vect[j]==1){
                    p_vect2.push_back(1);
                } else {p_vect2.push_back(0);}
            }

            std::vector<int> x_vect2; x_vect2.clear();
            for (int j=0; j<m; j++) {
                if (adj_matrix[i][j]==1 && x_vect[j]==1){
                    x_vect2.push_back(1);
                } else {x_vect2.push_back(0);}
            }


            BronKerbosch2(r_vect2, p_vect2, x_vect2);

            p_vect[i]=0;
            x_vect[i]=1;
        }
    }
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/rvispecn17968.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    n = graphJson["n"].asInt();
    m = graphJson["m"].asInt();

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
    //compute complement intersection graph as an adjacency matrix
    //std::vector<std::vector<int>> adjacency_list;
    adjacency_list.clear();
    adj_matrix.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        adj_matrix.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {adj_matrix[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list[l].push_back(k);
                adj_matrix[l].push_back(1);
            } else {adj_matrix[l].push_back(0);}
        }
    }

    //determine degrees in complement of intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    degrees.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        degree_intersection_graph.push_back(std::make_pair(l,temp));
    }

    //https://en.wikipedia.org/wiki/Bron%E2%80%93Kerbosch_algorithm

    p_vec.clear(); r_vec.clear(); x_vec.clear();

    for (int i=0; i<m; i++){
        p_vec.push_back(1);
        r_vec.push_back(0);
        x_vec.push_back(0);
    }

    max_clique_size=0;

    BronKerbosch2(r_vec, p_vec, x_vec);


 

}