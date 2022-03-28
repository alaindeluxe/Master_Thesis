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

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/vispecn2518.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int k=70;

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
    std::vector<std::vector<int>> intersection_graph;
    std::vector<int> color_assignment_intersection_graph;
    intersection_graph.clear(); color_assignment_intersection_graph.clear();
    for (int i=0; i<m; i++){
        intersection_graph.push_back({});
        for (int j=0; j<m; j++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (i==j) {intersection_graph[i].push_back(0);}
            else if (doIntersect(edge_i[i], edge_j[i], edge_i[j], edge_j[j])){
                intersection_graph[i].push_back(1);
            } else {intersection_graph[i].push_back(0);}
        }
    }

    // assign colors randomly to edges
    std::vector<int> edge_colors; edge_colors.clear();
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(0,k-1); // guaranteed unbiased

    for (int i=0; i<m; i++){
        int random_integer = uni(rng);
        edge_colors.push_back(random_integer);
        //cout << random_integer << "\n";
    }


    //compute f
    int f=0;
    for (int i=0; i<m; i++){
        for (int j=i+1; j<m; j++){
            if (intersection_graph[i][j]==1 && edge_colors[j]==edge_colors[i]) {f++;}
        }
    }

    //set up forbidden steps table
    std::vector<std::vector<int>> forbidden_steps; forbidden_steps.clear();
    for (int i=0; i<m; i++){
        forbidden_steps.push_back({});
        for (int j=0; j<k; j++){
            forbidden_steps[i].push_back(0);
        }
    }

    //while f>0, find best change of coloring
    int niter=0;
    while (f>0 && niter<10000){

        std::vector<std::vector<int>> possible_steps; possible_steps.clear();
        for (int i=0; i<m; i++){
            possible_steps.push_back({});
            for (int j=0; j<k; j++){
                if (forbidden_steps[i][j]==0){
                    possible_steps[i].push_back(0);
                } else {
                    possible_steps[i].push_back(-99999999);
                }
            }
        }

        for (int i=0; i<m; i++){
            for (int j=i+1; j<m; j++){
                if (intersection_graph[i][j]==1 && edge_colors[i]!=edge_colors[j]){
                    possible_steps[i][edge_colors[j]]--;
                    possible_steps[j][edge_colors[i]]--;
                } else if (intersection_graph[i][j]==1 && edge_colors[i]==edge_colors[j]){
                    for (int l=0; l<edge_colors[i]; l++){
                        possible_steps[i][l]++;
                        possible_steps[j][l]++;
                    }
                    for (int l=edge_colors[i]+1; l<k; l++){
                        possible_steps[i][l]++;
                        possible_steps[j][l]++;
                    }
                }

            }
        }

        int best_i, best_j, best_temp;
        best_temp=-99999999;

        for (int i=0; i<m; i++){
            for (int j=0; j<k; j++){
                if (possible_steps[i][j]>best_temp){
                    best_i=i; best_j=j; best_temp=possible_steps[i][j];
                }

            }
        }

        edge_colors[best_i]=best_j;
        f=f-best_temp;
        forbidden_steps[best_i][best_j]=f+niter+5;//std::ceil(5+0.6*f) or + random(0,10);

        cout << "f = " << f << "\n"; //now it stays at f=199, 201, 211

        for (int i=0; i<m; i++){
            for (int j=0; j<k; j++){
                forbidden_steps[i][j]=std::max(0, forbidden_steps[i][j]-1);
            }
        }   

        niter++;

    }

    int colors_counter=k;

    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
        for (int l=0; l<m; l++){
            if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[k]==edge_colors[l]){
                    valid_color_assignment=false;
                    //cout << "edge " << k << " and edge " << l << " have the same color, but shouldn't" << "\n";
                }
            }
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

     ofstream o("tabucol_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in TabuCol Algorithm: " << colors_counter << "\n";

}