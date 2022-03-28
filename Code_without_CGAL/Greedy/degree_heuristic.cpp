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

std::vector<int> degree;

bool custome_compare(const std::pair<int, int> &p1, const std::pair<int, int> &p2)
{
    return p1.second > p2.second;
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("cgshop_2022_examples_01/example-instances-sqrm/sqrm_5K_2.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();

    std::vector<int> x, y, edge_i, edge_j;
    x.clear(); y.clear();
    for (int i=0; i<n; i++){
        x.push_back(graphJson["x"][i].asInt());
        y.push_back(graphJson["y"][i].asInt());
    }

    std::vector<std::pair<int, int>> edges; //edge id's start at 0
    edges.clear();edge_i.clear();edge_j.clear();

    for (int i=0; i<m; i++){
        edge_i.push_back(graphJson["edge_i"][i].asInt());
        edge_j.push_back(graphJson["edge_j"][i].asInt());
        edges.push_back(std::make_pair(edge_i[i], edge_j[i]));
    }

    //keep vector with possible colors
    std::vector<int> possible_colors; possible_colors.clear();
    for (int i=0; i<n; i++){
        possible_colors.push_back(0); //0 means not used yet, 1 means used
    }



    //compute degrees of vertices
    degree.clear();
    for (int i=0; i<n; i++){
        degree.push_back(0);
    }

    for (int i=0; i<m; i++){
        degree[edges[i].first]++;
        degree[edges[i].second]++;
    }

    //create vector with pairs [vertex id, degree]
    std::vector<std::pair<int, int>> vertex_degree_pair; vertex_degree_pair.clear();
    for (int i=0; i<n; i++){
        vertex_degree_pair.push_back(std::make_pair(i, degree[i]));
    }
    //sort vector by ranking pairs from highest degree to lowest degree
    std::sort( std::begin(vertex_degree_pair), std::end(vertex_degree_pair), custome_compare );

    //determine vertex order
    std::vector<int> vertex_order; vertex_order.clear();
    for (int i=0; i<n; i++){
        vertex_order.push_back(vertex_degree_pair[i].first);
    }

    //determine edge order
    std::vector<int> used_edges; used_edges.clear();
    for (int i=0; i<m; i++){
        used_edges.push_back(0);
    }
    std::vector<std::pair<int,int>> edge_order; edge_order.clear();
    for (int i=0; i<n; i++){
        for (int j=0; j<m; j++){
            if (((vertex_order[i]==edges[j].first) || (vertex_order[i]==edges[j].second)) && (used_edges[j]==0)){
                edge_order.push_back(edges[j]);
                used_edges[j]=1;
            }
        }
    }

    //perform star algorithm, starting with the vertex with highest degree
    std::vector<int> edge_colors; edge_colors.clear();
    int colors_counter=0;

    //color edges incident to the first vertex with same color
    //go over edges incident to vertex i, for each edge, try out color 0,1,2,3,... until a color fits
    int max_color=0;
    for (int j=0; j<m; j++){
        int current_color=0;
        for (int k=0; k<j; k++){
            //find smallest color that fits
        }
        edge_colors.push_back(current_color);
        
    }

    colors_counter=max_color+1;

    //determine final color assignments
    std::vector<int> final_color_assignment; final_color_assignment.clear();
    for (int i=0; i<m; i++){
            int flag=0; int j=0;
            while ((flag==0) && (j<m)){
                if ((edges[i].first==edge_order[j].first) && (edges[i].second==edge_order[j].second)){
                    flag=1;
                } else {
                    j++;
                }
            }
            final_color_assignment.push_back(edge_colors[j]);
    }

    //write solution JSON

    Json::Value solutionjson;
    solutionjson["type"] = "Solution_CGSHOP2022";
    solutionjson["instance"] = graphJson["id"];
    solutionjson["num_colors"] = colors_counter;
    
    Json::Value vec(Json::arrayValue);
    for (int i=0; i<m; i++){
        vec.append(Json::Value(final_color_assignment[i]));
    }
    solutionjson["colors"] = vec;

    ofstream o("degree_heuristic_solution.json");
    o << std::setw(4) << solutionjson << std::endl;

}