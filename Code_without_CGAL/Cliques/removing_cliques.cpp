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

    ifstream file("instances/visp26405.instance.json");
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
    std::vector<std::vector<int>> adjacency_list, adjacency_list_2;
    adjacency_list.clear(); adjacency_list_2.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        adjacency_list_2.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list_2[l].push_back(k);
            } else {adjacency_list[l].push_back(k);}
        }
    }

    //determine degrees in complement of intersection graph
    std::vector<std::pair<int,int>> degree_complement_intersection_graph; degree_complement_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    std::vector<int> in_subgraph; in_subgraph.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        in_subgraph.push_back(1);
        degree_complement_intersection_graph.push_back(std::make_pair(l,temp));
    }

    std::vector<int> edge_colors; edge_colors.clear();
    for (int i=0; i<m; i++){
        edge_colors.push_back(-1);
    }

    //Remove cliques from the remains of the complement intersection graph until we colored all vertices
    int graph_size=m;
    int colors_counter=0;

    while(graph_size>0){

        //find next clique
        bool clique_found=false;
        while (graph_size>0 && clique_found==false){
            //find vertex v with smallest degree in subgraph
            int min_degree=graph_size;
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

        //color vertices of clique with colors_counter
        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1){
                edge_colors[i]=colors_counter;
            }
        }

        colors_counter++;

        //reset remaining graph
        graph_size=0;
        for (int i=0; i<m; i++){
            if (edge_colors[i]==-1){
                graph_size++;
                in_subgraph[i]=1;
            } else {
                in_subgraph[i]=0;
            }
        }

        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1){
                degrees[i]=0;
                for (int j=0; j<adjacency_list[i].size(); j++){
                    if (in_subgraph[adjacency_list[i][j]]==1){
                        degrees[i]++;
                    }
                }
            }
        }

    }




    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int j=0; j<m; j++){
        for (int l=0; l<adjacency_list_2[j].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[j]==edge_colors[adjacency_list_2[j][l]]){
                    valid_color_assignment=false;
                    cout << "edge " << j << " and edge " << l << " have the same color, but shouldn't" << "\n";
                }
            //}
        }
    }

    cout << "Color Assignment is valid: " << valid_color_assignment << "\n";

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

     ofstream o("removing_cliques_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in Removing_Cliques Algorithm: " << colors_counter << "\n";

}