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

int vertex_counter;
std::vector<bool> visited;
std::vector<int> vertex_order;
std::vector<std::vector<int>> adjacency_list;

void dfs(int i){
    visited[i]=true;
    vertex_order[i]=vertex_counter;
    vertex_counter++;

    //cout << vertex_counter << " ";

    for (int j=0; j<adjacency_list[i].size(); j++){
        if (visited[adjacency_list[i][j]]==false){
            dfs(adjacency_list[i][j]);
        }
    }
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/reecn3382.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int colors_max=1000;

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
    //std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        //degree_intersection_graph.push_back(std::make_pair(l,temp));
        degrees.push_back(temp);
    }

    //sort edges from highest degree to lowest degree
    //sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);

    std::vector<int> edge_colors;
    int colors_counter=0;

    for (int i=0; i<m; i++){ // m=1
        //determine vertex order accoring to DFS
        vertex_order.clear();
        for (int j=0; j<m; j++){
            vertex_order.push_back(-1);
        }

        vertex_counter=0;
        visited.clear();
        for (int j=0; j<m; j++){
            visited.push_back(false);
        }

        dfs(i);

        edge_colors.clear();
        std::vector<std::pair<int,int>> vertex_order_dfs; vertex_order_dfs.clear();

        for (int j=0; j<m; j++){
            //cout << vertex_order[j] << "\n";
            edge_colors.push_back(-1);
            vertex_order_dfs.push_back(std::make_pair(j, vertex_order[j]));
        }

        sort(vertex_order_dfs.begin(), vertex_order_dfs.end(), myComp_low_to_high);

        colors_counter=0;

        for (int j=0; j<m; j++){
            int current_vertex=vertex_order_dfs[j].first;

            std::vector<int> possible_colors; possible_colors.clear();
            for (int k=0; k<colors_max; k++){
                possible_colors.push_back(0);
            }


            for (int k=0; k<adjacency_list[current_vertex].size(); k++){
                if (vertex_order[adjacency_list[current_vertex][k]]<j){
                    possible_colors[edge_colors[adjacency_list[current_vertex][k]]]=1;
                }

                int t=0;
                while (possible_colors[t]==1){t++;}
                edge_colors[current_vertex]=t;

            }

        }

        for (int j=0; j<m; j++){
            if (edge_colors[j]+1>colors_counter) {colors_counter=edge_colors[j]+1;}
        }

        cout << "Colors Counter: " << colors_counter << "\n";

    }






    //sort color classes such that the biggest color classes have small color
    std::vector<int> color_class_size; color_class_size.clear();
    for (int i=0; i<colors_counter; i++){
        color_class_size.push_back(0);
    }

    for (int i=0; i<m; i++){
        color_class_size[edge_colors[i]]++;
    }

    std::vector<std::pair<int,int>> colors_with_size; colors_with_size.clear();

    for (int i=0; i<colors_counter; i++){
        colors_with_size.push_back(std::make_pair(i, color_class_size[i]));
    }

    sort(colors_with_size.begin(),colors_with_size.end(), myComp_high_to_low);

    std::vector<int> color_mapping; color_mapping.clear();
    for (int i=0; i<colors_counter; i++){
        color_mapping.push_back(0);
    }

    for (int i=0; i<colors_counter; i++){
        color_mapping[colors_with_size[i].first]=i;
    }

    for (int i=0; i<m; i++){
        edge_colors[i]=color_mapping[edge_colors[i]];
    }

    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
        for (int l=0; l<adjacency_list[k].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[k]==edge_colors[adjacency_list[k][l]]){
                    valid_color_assignment=false;
                    cout << "edge " << k << " and edge " << adjacency_list[k][l] << " have the same color, but shouldn't" << "\n";
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

     ofstream o("dfs_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in DFS Algorithm: " << colors_counter << "\n";

}