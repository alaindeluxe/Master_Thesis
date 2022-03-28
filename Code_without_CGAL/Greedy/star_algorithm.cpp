#include <iostream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>

using namespace std;

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/visp73369.instance.json");
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

    //perform star algorithm
    std::vector<int> edge_colors; edge_colors.clear();
    int colors_counter=0;
    for (int i=0; i<m; i++){
        if (edges[i].first<edges[i].second){
            if (possible_colors[edges[i].first]==0){
                colors_counter++;
                possible_colors[edges[i].first]=1;
            }
            edge_colors.push_back(edges[i].first);

        } else {
            if (possible_colors[edges[i].second]==0){
                colors_counter++;
                possible_colors[edges[i].second]=1;
            }
            edge_colors.push_back(edges[i].second);
        }
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

    ofstream o("star_solution.json");
    o << std::setw(4) << solutionjson << std::endl;

}