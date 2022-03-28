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

    ifstream file("instances/vispecn2518.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int maxColors=1200;

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

    //determine degrees in intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degree_intersection_graph.push_back(std::make_pair(l,temp));
        degrees.push_back(temp);
    }

    //sort edges from highest degree to lowest degree
    sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);

    int current_vertex=degree_intersection_graph[0].first;

    int colors_counter=0;
    //initialize all edge colors to 0
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> corrected; corrected.clear();
    std::vector<int> highest_neighbor_color; highest_neighbor_color.clear();
    for (int k=0; k<m; k++){
        edge_colors.push_back(0);
        corrected.push_back(0);
        highest_neighbor_color.push_back(0);
    }

    std::vector<int> bad_crossings=degrees;
    int maxBadCrossings=1;



    //we now take the edge with the most "bad" crossings" and correct the color of this edge
    while (maxBadCrossings>0){

        //cout << "i=" << i << "\n";

        std::vector<int> possible_colors; possible_colors.clear();
        for (int j=0; j<maxColors; j++){
            possible_colors.push_back(0);
        } 
        for (int j=0; j<adjacency_list[current_vertex].size(); j++){
            possible_colors[edge_colors[adjacency_list[current_vertex][j]]]=1;
        }
        int t=0;
        while (possible_colors[t]==1) {t++;}

        for (int j=0; j<adjacency_list[current_vertex].size(); j++){
            highest_neighbor_color[adjacency_list[current_vertex][j]]=max(t,highest_neighbor_color[adjacency_list[current_vertex][j]]);
            if (edge_colors[current_vertex]==edge_colors[adjacency_list[current_vertex][j]]){
                bad_crossings[adjacency_list[current_vertex][j]]=max(0,bad_crossings[adjacency_list[current_vertex][j]]-1);
            }
        }

        edge_colors[current_vertex]=t;
        bad_crossings[current_vertex]=0;
        corrected[current_vertex]=1;



        //find next vertex
        maxBadCrossings=0;
        int max_highest_neighbor_color=0;

        for (int j=0; j<m; j++){
            if ((bad_crossings[j]>maxBadCrossings) && (corrected[j]==0)){
                maxBadCrossings=bad_crossings[j];
                //current_vertex=j;
            }
            if ((highest_neighbor_color[j]>max_highest_neighbor_color) && (corrected[j]==0)){
                max_highest_neighbor_color=highest_neighbor_color[j];
                current_vertex=j;
            }
        }

        //i++;
    }

    colors_counter=0;
    for (int j=0; j<m; j++){
        if (edge_colors[j]+1>colors_counter){
            colors_counter=edge_colors[j];
        }
    }









    //sort color classes such that the biggest color classes have small color
    // std::vector<int> color_class_size; color_class_size.clear();
    // for (int i=0; i<colors_counter; i++){
    //     color_class_size.push_back(0);
    // }

    // for (int i=0; i<m; i++){
    //     color_class_size[edge_colors[i]]++;
    // }

    // std::vector<std::pair<int,int>> colors_with_size; colors_with_size.clear();

    // for (int i=0; i<colors_counter; i++){
    //     colors_with_size.push_back(std::make_pair(i, color_class_size[i]));
    // }

    // sort(colors_with_size.begin(),colors_with_size.end(), myComp_high_to_low);

    // std::vector<int> color_mapping; color_mapping.clear();
    // for (int i=0; i<colors_counter; i++){
    //     color_mapping.push_back(0);
    // }

    // for (int i=0; i<colors_counter; i++){
    //     color_mapping[colors_with_size[i].first]=i;
    // }

    // for (int i=0; i<m; i++){
    //     edge_colors[i]=color_mapping[edge_colors[i]];
    // }

    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
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

     ofstream o("correcting_crossings_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in CC Algorithm: " << colors_counter << "\n";

}