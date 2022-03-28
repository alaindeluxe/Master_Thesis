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

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/sqrp12451.instance.json");
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
    //compute complement intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list, adj_matrix; //adjacency_list_2;
    adjacency_list.clear(); adj_matrix.clear(); //adjacency_list_2.clear();
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

    //determine degrees in intersection graph
    //std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    //std::vector<int> degrees; degrees.clear();
    //for (int l=0; l<m; l++){
        //int temp=adjacency_list[l].size();
        //degrees.push_back(temp);
        //degree_intersection_graph.push_back(std::make_pair(l,temp));
    //}


    //build star intersection graph (adjacency matrix)
    std::vector<std::vector<int>> star_intersection_graph; star_intersection_graph.clear();
    for (int i=0; i<n; i++){
        star_intersection_graph.push_back({});
        for (int j=0; j<n; j++){
            star_intersection_graph[i].push_back(0);
        }
    }

    for (int i=0; i<m; i++){
        for (int j=0; j<adjacency_list[i].size(); j++){
            int k=adjacency_list[i][j];
            int i_i=edge_i[i];
            int i_j=edge_j[i];
            int k_i=edge_i[k];
            int k_j=edge_j[k];

            star_intersection_graph[i_i][k_i]=1;
            star_intersection_graph[i_i][k_j]=1;
            star_intersection_graph[i_j][k_i]=1;
            star_intersection_graph[i_j][k_j]=1;

            star_intersection_graph[k_i][i_i]=1;
            star_intersection_graph[k_j][i_i]=1;
            star_intersection_graph[k_i][i_j]=1;
            star_intersection_graph[k_j][i_j]=1;
        }
    }

    //build adjacency_list
    std::vector<std::vector<int>> s_adjacency_list; s_adjacency_list.clear();
    for (int i=0; i<n; i++){
        s_adjacency_list.push_back({});
        for (int j=0; j<n; j++){
            if (star_intersection_graph[i][j]==1){
                s_adjacency_list[i].push_back(j);
            }
        }
    }

}