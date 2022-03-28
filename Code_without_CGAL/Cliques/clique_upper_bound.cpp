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
#include <map>

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

    ifstream file("instances/visp73369.instance.json");
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
    std::vector<std::vector<int>> adjacency_list, adj_matrix;
    adjacency_list.clear(); adj_matrix.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        adj_matrix.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(0), otherwise push_back(1)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {adj_matrix[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list[l].push_back(k);
                adj_matrix[l].push_back(1);
            } else {adj_matrix[l].push_back(0);}
        }
    }

    //determine degrees in intersection graph
    std::vector<int> degrees; degrees.clear();
    std::vector<int> max_clique_size; max_clique_size.clear();

    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        max_clique_size.push_back(0);
    }

    //for each edge, determine the maximum number of "clique edges" which can intersect this edge

    for (int i=0; i<m; i++){
        int vertex_counter=0;
        int left_vertex_counter=0;
        int right_vertex_counter=0;
        std::vector<int> cons_vertices; cons_vertices.clear();
        for (int j=0; j<n; j++) {cons_vertices.push_back(0);}

        for (int j=0; j<adjacency_list[i].size(); j++){

            int c_e=adjacency_list[i][j];
            if (cons_vertices[edge_i[c_e]]==0) {
                vertex_counter++;
                if (ccw(edge_i[j], edge_j[j], edge_i[c_e])){left_vertex_counter++;} else {right_vertex_counter++;}

            }
            cons_vertices[edge_i[c_e]]=1;

            if (cons_vertices[edge_j[c_e]]==0) {
                vertex_counter++;
                if (ccw(edge_i[j], edge_j[j], edge_j[c_e])){left_vertex_counter++;} else {right_vertex_counter++;}
            }
            cons_vertices[edge_j[c_e]]=1;


        }

        //need to compute maximum matching to maybe get a better upper bound

        max_clique_size[i]=min(left_vertex_counter, right_vertex_counter);

    }

    std::sort(max_clique_size.begin(), max_clique_size.end());
    std::sort(degrees.begin(), degrees.end());

    for (int i=0; i<m; i++){
        std::cout << max_clique_size[i] << " " << degrees[i] << "\n";
    }

    int upper_bound=0;

    while (upper_bound<=max_clique_size[m-1-upper_bound]){upper_bound++;}


    std::cout << "\n";
    

    //the resulting independent set size is the size of one of the cliques in the intersection graph
    cout << "Upper Bound for the largest clique: " << upper_bound << "\n";

}