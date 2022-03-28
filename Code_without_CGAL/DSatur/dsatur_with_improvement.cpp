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

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/sqrpecn3218.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int k=140;

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
    for (int l=0; l<m; l++){
        intersection_graph.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {intersection_graph[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                intersection_graph[l].push_back(1);
            } else {intersection_graph[l].push_back(0);}
        }
    }

    //determine degrees in intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    for (int l=0; l<m; l++){
        int temp=0;
        for (int k=0; k<m; k++){
            if (intersection_graph[l][k]==1) {temp++;}
        }
        degree_intersection_graph.push_back(std::make_pair(l,temp));
        degrees.push_back(temp);
    }

    //sort edges from highest degree to lowest degree
    sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);

    //initaialize saturation
    std::vector<int> saturation; saturation.clear();//saturation of a vertex
    std::vector<std::vector<int>> neighbors_colors; neighbors_colors.clear();//neighbors_colors[i][j]==1 if vertex i has a neighbors with color j
    for (int i=0; i<m; i++){
        saturation.push_back(0);
    }
    for (int i=0; i<m; i++){
        neighbors_colors.push_back({});
        for (int j=0; j<k; j++){
            neighbors_colors[i].push_back(0);
        }
    }

    int maxSat=0;
    int maxDegree=0;
    int maxSat_vertex=degree_intersection_graph[0].first;
    int i=0;

    int colors_counter=0;
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> used_colors; used_colors.clear();//contains all colors that have already been assigned
    for (int k=0; k<m; k++){
        edge_colors.push_back(-1);
    }

    while(i<m){
        int current_vertex=maxSat_vertex;
        std::vector<int> possible_colors; possible_colors.clear();
        for (int j=0; j<k; j++) {possible_colors.push_back(0);}
        for (int j=0; j<m; j++){
            if (intersection_graph[current_vertex][j]==1 && edge_colors[j]>=0){
                possible_colors[edge_colors[j]]=1;
            }
        }
        int t=0;
        while (t<used_colors.size() && possible_colors[used_colors[t]]==1) {t++;} 
        if (t<used_colors.size()){
            edge_colors[current_vertex]=used_colors[t];
            for (int j=0; j<m; j++){
                if (intersection_graph[current_vertex][j]==1 && neighbors_colors[j][used_colors[t]]==0){
                    neighbors_colors[j][used_colors[t]]=1;
                    saturation[j]++;
                }
            }
        } else {
            t=0;
            while (possible_colors[t]>0) {t++;}
            edge_colors[current_vertex]=t;
            used_colors.push_back(t);
            colors_counter++;
            for (int j=0; j<m; j++){
                if (intersection_graph[current_vertex][j]==1 && neighbors_colors[j][t]==0){
                    neighbors_colors[j][t]=1;
                    saturation[j]++;
                }
            }
        }

        //find vertex with highest saturation
        maxSat=0; maxDegree=0;
        for (int j=0; j<m; j++){
            if (edge_colors[j]==-1){
                if (saturation[j]>maxSat){
                    maxSat_vertex=j;
                    maxSat=saturation[j];
                    maxDegree= degrees[j];
                } else if (saturation[j]==maxSat && degrees[j]>maxDegree){
                    maxSat_vertex=j;
                    maxSat=saturation[j];
                    maxDegree= degrees[j];
                }
            }

        }

        i++;
    }




    //redefine used_colors in order to use the improevement algorithm
    used_colors.clear();
    for (int i=0; i<n; i++){
        used_colors.push_back(0);
    }
    for (int i=0; i<m; i++){
        used_colors[edge_colors[i]]++;
    }


    //use improvement algorithm

    //sort vertices of intersection graph by their assigned color, from highest to lowest

    std::vector<std::pair<int,int>> vertex_with_color; vertex_with_color.clear();

    for (int i=0; i<m; i++){
        vertex_with_color.push_back(std::make_pair(i,edge_colors[i]));
    }

    sort(vertex_with_color.begin(),vertex_with_color.end(), myComp_high_to_low);

    //for (int l=0; l<m; l++){
    //    cout << vertex_with_color[l].first << " " << vertex_with_color[l].second << "\n";
    //}

    //we now go through the list vertex_with_color and for each vertex, we try to assign it a smaller color

    for (int i=0; i<m; i++){//b=m
        //find neighbors of vertex i and sort them by color
        std::vector<std::pair<int,int>> current_neighbors_with_color; current_neighbors_with_color.clear();
        for (int j=0; j<m; j++){
            if (i!=j && intersection_graph[i][j]==1){
                current_neighbors_with_color.push_back(std::make_pair(j, edge_colors[j]));
            }
        }
        sort(current_neighbors_with_color.begin(), current_neighbors_with_color.end(), myComp_high_to_low);

        int first_neighbor_index=0;
        int last_neighbor_index=0;

        while (last_neighbor_index<current_neighbors_with_color.size()){

            int current_color=current_neighbors_with_color[first_neighbor_index].second;

            //check whether current_color works for vertex i
            bool current_color_works=true;
            for (int k=0; k<first_neighbor_index; k++){
                if (edge_colors[current_neighbors_with_color[k].first]==current_color) {current_color_works=false;}
            }

            //find all next neighbors that have the same color

            while (current_color==edge_colors[current_neighbors_with_color[last_neighbor_index].first] && last_neighbor_index+1<current_neighbors_with_color.size()) {
                last_neighbor_index++;
            }
            if (last_neighbor_index==current_neighbors_with_color.size()-1 && current_color==edge_colors[current_neighbors_with_color[last_neighbor_index].first]){

            } else {last_neighbor_index--;}

            //cout << "Currently checking i=" << i << " from " << first_neighbor_index << " to " << last_neighbor_index << "\n";
            //cout << "Number of neighbors of i: " << current_neighbors_with_color.size() << "\n";

            if (current_color_works){
        
            //check whether we can change the colors from first_neighbor to last_neighbor (that have the same color) to a different color between 0 and edge_colors[i].
            std::vector<int> best_colors; best_colors.clear();//best_colors[i] shows the best possible new color assignment for i
            for (int k=0; k<=last_neighbor_index-first_neighbor_index; k++){
                best_colors.push_back(-1);
            }
            for (int k=first_neighbor_index; k<=last_neighbor_index; k++){
                //find possible colors for this vertex that are smaller than edge_colors[i]
                std::vector<int> possible_colors; possible_colors.clear();
                for (int l=0; l<n; l++){
                    possible_colors.push_back(0); //possible_colors[i]==1 means that color i is not an option
                }
                //cout << "Checking neighbor " << k << "\n";
                int current_vertex = current_neighbors_with_color[k].first;
                for (int l=0; l<m; l++){
                    if (current_vertex!=l && intersection_graph[current_vertex][l]==1){
                        possible_colors[edge_colors[l]]=1;
                    }
                }
                int t=0;
                while (possible_colors[t]>0 || t==current_color) {t++;}
                if (t<edge_colors[i]) {best_colors[k-first_neighbor_index]=t;}

            }

            //if every vertex has a best_color, we can change the colors of the vertices

            bool everybody_best_color=true;
            for (int k=0; k<=last_neighbor_index-first_neighbor_index; k++){
                if (best_colors[k]==-1) {everybody_best_color=false;}
            }

            if (everybody_best_color){
                cout << "Updating colors for i=" << i << ", k between " << first_neighbor_index << " " << last_neighbor_index << "\n";
                //update color of vertex i
                used_colors[edge_colors[i]]--;
                if (used_colors[edge_colors[i]]==0){
                    colors_counter--;
                }
                edge_colors[i]=current_color;
                used_colors[current_color]++;
                if (used_colors[current_color]==1){
                    colors_counter++;
                }
                //update color of current_neighbors
                for (int k=first_neighbor_index; k<=last_neighbor_index; k++){
                    int current_vertex = current_neighbors_with_color[k].first;
                    used_colors[edge_colors[current_vertex]]--;
                    if (used_colors[edge_colors[current_vertex]]==0){
                        colors_counter--;
                    }
                    edge_colors[current_vertex]=best_colors[k-first_neighbor_index];
                    used_colors[edge_colors[current_vertex]]++;
                    if (used_colors[edge_colors[current_vertex]]==1){
                        colors_counter++;
                    }
                }
            }

            }

            //else we move on

            first_neighbor_index=last_neighbor_index+1;
            last_neighbor_index=first_neighbor_index;

        }
    
    }
    

    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
        for (int l=0; l<m; l++){
            if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[k]==edge_colors[l]){
                    valid_color_assignment=false;
                    cout << "edge " << k << " and edge " << l << " have the same color, but shouldn't" << "\n";
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

     ofstream o("dsatur_impr_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in DSatur w.Impr. Algorithm: " << colors_counter << "\n";

}