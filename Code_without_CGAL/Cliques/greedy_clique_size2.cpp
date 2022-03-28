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
    //we want to find a large clique in the intersection graph
    //we do this by finding a large independent set in the complement of the intersection graph
    //compute complement of intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list;
    adjacency_list.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(0), otherwise push_back(1)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){

            } else {adjacency_list[l].push_back(k);}
        }
    }

    //determine degrees in complement of intersection graph
    //std::vector<std::pair<int,int>> degree_complement_intersection_graph; degree_complement_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    std::vector<int> in_subgraph; in_subgraph.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        in_subgraph.push_back(1);
        //degree_complement_intersection_graph.push_back(std::make_pair(l,temp));
    }

    int graph_size=m;
    bool clique_found=false;
    while (graph_size>0 && clique_found==false){
        //find vertex v with smallest degree in subgraph
        int min_degree=m;
        int v=0;
        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1 && degrees[i]<min_degree){
                v=i;
                min_degree=degrees[i];
            }
        }

        //if v has degree graph_size-1: clique_found=true
        if (min_degree==graph_size-1) {clique_found=true;}
        else {
            in_subgraph[v]=0;
            graph_size--;
            for (int i=0; i<adjacency_list[v].size(); i++){
                if (in_subgraph[adjacency_list[v][i]]==1){
                    degrees[adjacency_list[v][i]]--;
                }
            }
        }
    }

    //check that all vertices which are in_subgraph actually form a clique
    bool clique_check=true;
    for (int i=0; i<m; i++){
        if (in_subgraph[i]==1){
            //cout << "Degree of i: " << degrees[i] << "\n";
            int v_counter=0;
            for (int j=0; j<adjacency_list[i].size(); j++){
                if (in_subgraph[adjacency_list[i][j]]==1) {v_counter++;}
            }
            if (v_counter!=graph_size-1) {clique_check=false;}
        }
    }
    

    //the resulting independent set size is the size of one of the cliques in the intersection graph
    cout << "We found a valid clique in the complement graph: " << clique_check << "\n";
    cout << "Greedy Clique Size in the complement graph: " << graph_size << "\n";
    cout << "Greedy Clique Size in the actual graph: " << m/graph_size << "\n";

}