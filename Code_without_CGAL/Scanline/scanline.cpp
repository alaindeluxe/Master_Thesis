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

    ifstream file("instances/rvisp5013.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int max_colors=207;

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
    //compute intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list;
    adjacency_list.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list[l].push_back(k);
            } else {}
        }
    }

    std::vector<int> edge_colors; edge_colors.clear();
    for (int i=0; i<m; i++){
        edge_colors.push_back(-1);
    }

    //determine the order in which the scanline colors the edges
    std::vector<std::pair<int,int>> vertex_with_x; vertex_with_x.clear();
    for (int i=0; i<n; i++){
        vertex_with_x.push_back(std::make_pair(i, x[i]));
    }

    std::vector<std::pair<int,int>> vertex_with_y; vertex_with_y.clear();
    for (int i=0; i<n; i++){
        vertex_with_y.push_back(std::make_pair(i, y[i]));
    }

    //sort(vertex_with_x.begin(),vertex_with_x.end(), myComp_high_to_low);
    sort(vertex_with_y.begin(),vertex_with_y.end(), myComp_high_to_low);

    int colored_edges=0;

    while (colored_edges<m){

        for (int i=0; i<n; i++){

            //int current_vertex=vertex_with_x[i].first;
            int current_vertex=vertex_with_y[i].first;

            int flag=0;
            //find an edge incident to current_vertex which is not colored yet
            for (int j=0; j<m && flag==0; j++){
                if (edge_i[j]==current_vertex && edge_colors[j]==-1){
                    std::vector<int> possible_colors; possible_colors.clear();
                    for (int l=0; l<max_colors; l++){
                        possible_colors.push_back(0);
                    }
                    for (int l=0; l<adjacency_list[j].size(); l++){
                        if (edge_colors[adjacency_list[j][l]]>=0){
                            possible_colors[edge_colors[adjacency_list[j][l]]]=1;
                        }
                    }
                    int t=0;
                    while (possible_colors[t]==1) {t++;}
                    edge_colors[j]=t;
                    colored_edges++;
                    flag=1;
                } else if (edge_j[j]==current_vertex && edge_colors[j]==-1){
                    std::vector<int> possible_colors; possible_colors.clear();
                    for (int l=0; l<max_colors; l++){
                        possible_colors.push_back(0);
                    }
                    for (int l=0; l<adjacency_list[j].size(); l++){
                        if (edge_colors[adjacency_list[j][l]]>=0){
                            possible_colors[edge_colors[adjacency_list[j][l]]]=1;
                        }
                    }
                    int t=0;
                    while (possible_colors[t]==1) {t++;}
                    edge_colors[j]=t;
                    colored_edges++;
                    flag=1;
                }
            }
        
        }

    }

    int colors_counter=0;
    for (int i=0; i<m; i++){
        if (edge_colors[i]+1>colors_counter){
            colors_counter=edge_colors[i]+1;
        }
    }





    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
        if (edge_colors[k]==-1) {valid_color_assignment=0;}
        for (int l=0; l<adjacency_list[k].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[k]==edge_colors[adjacency_list[k][l]]){
                    valid_color_assignment=false;
                    cout << "edge " << k << " and edge " << adjacency_list[k][l] << " have the same color: " << edge_colors[k] << "\n";
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

     ofstream o("scanline_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in Scanline Algorithm: " << colors_counter << "\n";

}