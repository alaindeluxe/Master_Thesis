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

bool myComparison_complement(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second<b.second;
}

bool myComparison(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second>b.second;
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/reecn73116.instance.json");
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
    std::vector<std::vector<int>> intersection_graph;
    //std::vector<int> color_assignment_intersection_graph;
    intersection_graph.clear(); //color_assignment_intersection_graph.clear();

    std::vector<std::vector<int>> complement_intersection_graph;
    std::vector<int> color_assignment_complement_intersection_graph;
    complement_intersection_graph.clear(); color_assignment_complement_intersection_graph.clear();
    for (int l=0; l<m; l++){
        complement_intersection_graph.push_back({});
        intersection_graph.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(0), otherwise push_back(1)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {complement_intersection_graph[l].push_back(1);intersection_graph[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                complement_intersection_graph[l].push_back(0);intersection_graph[l].push_back(1);
            } else {complement_intersection_graph[l].push_back(1);intersection_graph[l].push_back(0);}
        }
    }


    //determine degrees in complement of intersection graph
    std::vector<std::pair<int,int>> degree_complement_intersection_graph; degree_complement_intersection_graph.clear();
    for (int l=0; l<m; l++){
        int temp=0;
        for (int k=0; k<m; k++){
            if (complement_intersection_graph[l][k]==1) {temp++;}
        }
        degree_complement_intersection_graph.push_back(std::make_pair(l,temp));
    }
    //determine degrees in intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    for (int l=0; l<m; l++){
        int temp=0;
        for (int k=0; k<m; k++){
            if (intersection_graph[l][k]==1) {temp++;}
        }
        degree_intersection_graph.push_back(std::make_pair(l,temp));
    }

    //sort edges from lowest degree to highest
    sort(degree_complement_intersection_graph.begin(),degree_complement_intersection_graph.end(), myComparison_complement);
    //sort edges from highest degree to lowest degree
    sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);

    //go over edges in the above order and mark them
    //remove its neighbors
    //continue this procedure until no vertex is left in the complement of the intersection graph

    int number_possible_edges=m;//number of edges we can still choose
    int l=0;
    std::vector<int> edge_set; edge_set.clear();
    //edge_set[x]==1 means that edge x is part of our independent set
    //edge_set[x]==0 means that edge x is not in the independent set and has not been deleted
    //edge_set[x]==-1 means that edge x is deleted

    int independent_set_size=0;
    for (int k=0; k<m; k++){
        edge_set.push_back(0);
    }

    while ((number_possible_edges>0) && (l<m)){
        int current_edge=degree_complement_intersection_graph[l].first;
        if (edge_set[current_edge]==0){
            edge_set[current_edge]=1;
            independent_set_size++;
            for (int k=0; k<m; k++){
                if (complement_intersection_graph[current_edge][k]==1) {
                    if (edge_set[k]==0) {number_possible_edges--;}
                    edge_set[k]=-1;
                }
            }
        }
        l++;
    }
    

    //the resulting independent set size is the size of one of the cliques in the intersection graph
    cout << "Greedy Clique Size: " << independent_set_size << "\n";

//-------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------


    //go over edges in the above order and color greedily
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> used_colors; used_colors.clear();
    for (int k=0; k<m; k++){
        edge_colors.push_back(-1);
    }
    for (int k=0; k<n; k++){
        used_colors.push_back(0);
    }

    for (int k=0; k<m; k++){
        int current_edge = degree_intersection_graph[k].first;
        std::vector<int> possible_colors; possible_colors.clear();
        for (int l=0; l<n; l++){
            possible_colors.push_back(0);
        }
        //possible_colors[x]==0 means that color x is still available for current_edge
        for (int l=0; l<m; l++){
            if (intersection_graph[current_edge][l]==1 && edge_colors[l]>=0){
                possible_colors[edge_colors[l]]=1;
            }
        }
        //find lowest color x such that possible_colors[x]==0
        int t=0;
        while (possible_colors[t]>0) {t++;}
        edge_colors[current_edge]=t;
        used_colors[t]=1;
    }

    int colors_counter=0;
    for (int k=0; k<n; k++){
        if (used_colors[k]==1) {colors_counter++;}
    }

    //write solution JSON

     Json::Value solutionjson;
     solutionjson["type"] = "Solution_CGSHOP2022";
     solutionjson["instance"] = graphJson["id"];
     solutionjson["num_colors"] = colors_counter;
    
     Json::Value vec(Json::arrayValue);
     for (int i=0; i<m; i++){
         vec.append(Json::Value(edge_colors[i]));
     }
     solutionjson["colors"] = vec;

     ofstream o("greedy_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in Greedy Algorithm: " << colors_counter << "\n";

}