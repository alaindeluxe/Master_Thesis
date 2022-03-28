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

    ifstream file("instances/sqrpecn73925.instance.json");
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

    //determine degrees in complement of intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    std::vector<int> in_subgraph; in_subgraph.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        in_subgraph.push_back(1);
        degree_intersection_graph.push_back(std::make_pair(l,temp));
    }

    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> partition;partition.clear();
    for (int i=0; i<m; i++){
        edge_colors.push_back(-1);
        partition.push_back(-1);
    }

    //Remove cliques from the remains of the complement intersection graph until we colored all vertices
    int graph_size=m;
    int colors_counter=0;
    int partition_counter=0;

    while(graph_size>0){

        std::cout << "graph size = " << graph_size << "\n";

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

        //make clique maximal
        for (int i=0; i<m; i++){
            //cout << i << "\n";
            if (in_subgraph[i]==0 && partition[i]==-1){
                bool in_clique=true;
                int c=graph_size;
                for (int j=0; j<adjacency_list[i].size(); j++){
                    if (in_subgraph[adjacency_list[i][j]]){
                        c--;
                    }
                }
                if (c>0) {in_clique=false;}
                if (in_clique){
                    in_subgraph[i]=1;
                    graph_size++;
                }
            }
        }

        std::cout << "Clique Size = " << graph_size << "\n";

        //assign vertices to partition
        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1 && partition[i]==-1){
                partition[i]=partition_counter;
            }
        }

        partition_counter++;

        //reset remaining graph
        graph_size=0;
        for (int i=0; i<m; i++){
            if (partition[i]==-1){
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

    //now that we have partitioned the graph into cliques, we make sure that partition 0 is the largets clique
    std::vector<int> partition_size; partition_size.clear();
    for (int i=0; i<partition_counter; i++){
        partition_size.push_back(0);
    }

    for (int i=0; i<m; i++){
        partition_size[partition[i]]++;
    }

    std::vector<std::pair<int,int>> partitions_with_size; partitions_with_size.clear();

    for (int i=0; i<partition_counter; i++){
        partitions_with_size.push_back(std::make_pair(i, partition_size[i]));
        //std::cout << i << " " << partition_size[i] << "\n";
    }

    sort(partitions_with_size.begin(),partitions_with_size.end(), myComp_high_to_low);

    std::vector<int> partition_mapping; partition_mapping.clear();
    for (int i=0; i<partition_counter; i++){
        partition_mapping.push_back(0);
    }

    for (int i=0; i<partition_counter; i++){
        partition_mapping[partitions_with_size[i].first]=i;
    }

    for (int i=0; i<m; i++){
        partition[i]=partition_mapping[partition[i]];
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    for (int i=0; i<partition_counter; i++){
        partition_size[i]=0;
    }

    for (int i=0; i<m; i++){
        partition_size[partition[i]]++;
    }

    // for (int i=0; i<partition_counter; i++){
    //     std::cout << i << " " << partition_size[i] << "\n";
    // }



    //color the vertices starting from partition 0 up to partition partition_counter-1
    for (int i=0; i<partition_counter; i++){
        //color vertices of partition i
        std::cout << "Looking at partition " << i << "\n";
        if (i==0){
            //give each vertex i partition 0 a different color
            for (int j=0; j<m; j++){
                if (partition[j]==0){
                    edge_colors[j]=colors_counter;
                    colors_counter++;
                }
            }
        } else {

            //build possible_vertex_color table
            std::vector<std::vector<int>> possible_vertex_color; possible_vertex_color.clear();
            for (int j=0; j<m; j++){
                possible_vertex_color.push_back({});
                for (int l=0; l<colors_counter; l++){
                    possible_vertex_color[j].push_back(0);//0 means available, 1 means color l is unavailable for vertex j
                }
            }
            for (int j=0; j<m; j++){
                if (partition[j]==i){
                    for (int k=0; k<adjacency_list[j].size(); k++){
                        if (partition[adjacency_list[j][k]]<i && edge_colors[adjacency_list[j][k]]>=0){
                            possible_vertex_color[j][edge_colors[adjacency_list[j][k]]]=1;
                        }
                    }
                }
            }

            //build n_possible_vertices and n_possible_colors

            std::vector<int> n_possible_vertices; n_possible_vertices.clear();
            for (int j=0; j<colors_counter; j++) {
                n_possible_vertices.push_back(0);
                for (int k=0; k<m; k++){
                    if (partition[k]==i && possible_vertex_color[k][j]==0){
                        n_possible_vertices[j]++;
                    }
                }
            }


            std::vector<int> n_possible_colors; n_possible_colors.clear();
            for (int j=0; j<m; j++){
                n_possible_colors.push_back(0);
                if (partition[j]==i){
                    for (int k=0; k<colors_counter; k++){
                        if (possible_vertex_color[j][k]==0){
                            n_possible_colors[j]++;
                        }
                    }
                }
            }

            for (int x=0; x<partition_size[i]; x++){

                //find vertex with smallest possible_colors
                int min_possible_colors=m;
                int current_vertex=0;
                for (int j=0; j<m; j++){
                    if (partition[j]==i && n_possible_colors[j]<min_possible_colors && edge_colors[j]==-1){
                        current_vertex=j;
                        min_possible_colors=n_possible_colors[j];
                    }
                }

                if (n_possible_colors[current_vertex]==0){
                    edge_colors[current_vertex]=colors_counter;
                    colors_counter++;
                } else {
                
                    //find color with smallest n_possible_vertices
                    //if there are more such vertices, find the one with smallest n_possible_colors

                    int min_possible_vertices=m;
                    std::vector<std::pair<int,int>> possible_vertices; possible_vertices.clear();
                    min_possible_colors=m;
                    int current_color=0;
                    current_vertex=0;
                    for (int k=0; k<colors_counter; k++){
                        if (n_possible_vertices[k]<min_possible_vertices && n_possible_vertices[k]>0){
                            possible_vertices.clear();
                            current_color=k;
                            min_possible_vertices=n_possible_vertices[k];
                            //find vertices which have smallest n_possible_colors
                            for (int j=0; j<m; j++){
                                if (possible_vertex_color[j][k]==0 && partition[j]==i){
                                    possible_vertices.push_back(std::make_pair(k,j));
                                }
                            }
                        } else if (n_possible_vertices[k]==min_possible_vertices && n_possible_vertices[k]>0){
                            for (int j=0; j<m; j++){
                                if (possible_vertex_color[j][k]==0 && partition[j]==i){
                                    possible_vertices.push_back(std::make_pair(k,j));
                                }
                            }
                        }
                    }

                    for (int j=0; j<possible_vertices.size(); j++){
                        if (n_possible_colors[possible_vertices[j].second]<min_possible_colors){
                            min_possible_colors=n_possible_colors[possible_vertices[j].second];
                            current_color=possible_vertices[j].first;
                            current_vertex=possible_vertices[j].second;
                        }
                    }

                    //assign current_vertex current_color
                    edge_colors[current_vertex]=current_color;
                    n_possible_vertices[current_color]=0;
                    n_possible_colors[current_vertex]=0;

                    //update possible_vertex_color table
                    for (int j=0; j<m; j++){
                        if (partition[j]==i && j!=current_vertex){
                            if (possible_vertex_color[j][current_color]==0){
                                possible_vertex_color[j][current_color]=1;
                                //update n_possible_colors
                                n_possible_colors[j]--;
                            }
                        }
                    }

                    //update n_possible_vertices
                    for (int k=0; k<colors_counter; k++){
                        if (possible_vertex_color[current_vertex][k]==0 && k!=current_color){
                            possible_vertex_color[current_vertex][k]=1;
                            n_possible_vertices[k]--;
                        }
                    }

                    //update n_possible_colors

                }


            }

        }
    }














    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int j=0; j<m; j++){
        for (int l=0; l<adjacency_list[j].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[j]==edge_colors[adjacency_list[j][l]]){
                    valid_color_assignment=false;
                    //cout << "edge " << j << " and edge " << adjacency_list[j][l] << " have the same color, but shouldn't" << "\n";
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

     cout << "Number of Colors in Conquering_Cliques Algorithm: " << colors_counter << "\n";

}