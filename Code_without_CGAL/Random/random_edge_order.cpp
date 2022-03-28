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
#include <iterator>

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
        //edges.push_back(std::make_pair(edge_i[i], edge_j[i]));
    }

    //perform algorithm
    //compute intersection graph as an adjacency matrix
    int crossings=0;
    std::vector<std::vector<int>> intersection_graph;
    std::vector<int> color_assignment_intersection_graph;
    intersection_graph.clear(); color_assignment_intersection_graph.clear();
    for (int l=0; l<m; l++){
        intersection_graph.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {intersection_graph[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                intersection_graph[l].push_back(1);crossings++;
            } else {intersection_graph[l].push_back(0);}
        }
    }
    crossings=crossings/2;
    cout << "Crossings in Graph: " << crossings << "\n";

    //determine random order of edges
    std::vector<int> edge_order; edge_order.clear();
    for (int k=0; k<m; k++){
        edge_order.push_back(k);
        //cout << edge_order[k] << " ";
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(edge_order.begin(), edge_order.end(), g); //works, I tested it!

    //go over each edge in the new order and give it the lowest possible color
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> used_colors; used_colors.clear();
    for (int k=0; k<m; k++){
        edge_colors.push_back(-1);//-1 means that corresponding edge doesn't have a color yet
    }
    for (int k=0; k<n; k++){
        used_colors.push_back(0);//0 means not used yet, 1 means at east one edge has the corresponding color
    }

    for (int k=0; k<m; k++){
        int current_edge = edge_order[k];
        std::vector<int> possible_colors; possible_colors.clear();
        for (int l=0; l<n; l++){
            possible_colors.push_back(0);//0 means color is still available, 1 means that it's already "taken"
        }
        for (int l=0; l<m; l++){
            if (intersection_graph[current_edge][l]==1 && edge_colors[l]>=0){
                possible_colors[edge_colors[l]]=1;
            }
        }
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

     ofstream o("random_edge_order_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in Greedy Algorithm: " << colors_counter << "\n";

}