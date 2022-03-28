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
       return a.second<b.second;
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
    //we want to find a large clique in the intersection graph
    //we do this by finding a large independent set in the complement of the intersection graph
    //compute complement of intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list, adj_matrix;
    adjacency_list.clear(); adj_matrix.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        adj_matrix.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(0), otherwise push_back(1)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {adj_matrix[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list[l].push_back(k);
                adj_matrix[l].push_back(1);
            } else {adj_matrix[l].push_back(0);}
        }
    }

    //determine degrees in complement of intersection graph
    std::vector<int> degrees; degrees.clear();
    for (int l=0; l<m; l++){
        degrees.push_back(0);
    }

    //go over all pairs of intersecting edges 
    //for each pair, determine the edges which intersect both edges in the pair -> form G' with these edges
    //find a clique of size c in G' -> we found a clique of size c+2 in the actual graph

    int max_clique_size=0;

    for (int i=0; i<m; i++){
        for (int j=i+1; j<m; j++){
            if (adj_matrix[i][j]==1){
                //form G'
                int graph_size=0;
                bool clique_found=false;
                for (int l=0; l<m; l++){
                    degrees[l]=0;
                }


                std::vector<int> in_subgraph; in_subgraph.clear();
                for (int l=0; l<m; l++){
                    if (adj_matrix[i][l]==1 && adj_matrix[j][l]==1){
                        in_subgraph.push_back(1); graph_size++;
                        for (int o=0; o<l; o++){
                            if (adj_matrix[l][o]==1 && in_subgraph[o]==1){
                                degrees[l]++; degrees[o]++;
                            }
                        }

                    } else {in_subgraph.push_back(0);}
                }

                //is this graph a clique?
                int min_degree=m;
                int v=0;
                for (int l=0; l<m; l++){
                    if (in_subgraph[l]==1 && degrees[l]<min_degree){
                        v=l;
                        min_degree=degrees[l];
                    }
                }

                if (min_degree==graph_size-1) {
                    clique_found=true;
                }


                //find clique in G'
                //for reecn2518: -5 gives 59, -2 gives 58
                while (graph_size>max_clique_size-5 && clique_found==false){//maybe we can change the -5? It does have an influence 
                    //find vertex v with smallest degree in subgraph
                    min_degree=m;
                    v=0;
                    for (int l=0; l<m; l++){
                        if (in_subgraph[l]==1 && degrees[l]<min_degree){
                            v=l;
                            min_degree=degrees[l];
                        }
                    }

                    if (min_degree==graph_size-1) {
                        clique_found=true;
                        //std::cout << "graph size = " << graph_size << "\n";
                    } else {
                        in_subgraph[v]=0;
                        graph_size--;
                        for (int l=0; l<adjacency_list[v].size(); l++){
                            if (in_subgraph[adjacency_list[v][l]]==1){
                                degrees[adjacency_list[v][l]]--;
                            }
                        }
                    }
                }

                //check whether we can add one of the vertices which is not in the subgraph, if we have indeed found clique
                if (clique_found==true){
                    for (int l=0; l<m; l++){
                        if (in_subgraph[l]==0){
                            bool in_clique=true;
                            int c=graph_size;
                            for (int o=0; o<adjacency_list[l].size(); o++){
                                if (in_subgraph[adjacency_list[l][o]]){
                                    c--;
                                }
                            }
                            if (c>0) {in_clique=false;}
                            if (in_clique){
                                in_subgraph[l]=1;
                                graph_size++;
                            }
                        }
                    }
                }

                // //check that all vertices which are in_subgraph actually form a clique
                // bool clique_check=true;
                // for (int l=0; l<m; l++){
                //     if (in_subgraph[l]==1){
                //         //cout << "Degree of i: " << degrees[i] << "\n";
                //         int v_counter=0;
                //         for (int o=0; o<adjacency_list[l].size(); o++){
                //             if (in_subgraph[adjacency_list[l][o]]==1) {v_counter++;}
                //         }
                //         if (v_counter!=graph_size-1) {
                //             clique_check=false;
                //         }
                //     }
                // }

                //if (clique_check==true) {cout << "Clique is correct" << "\n";} else {cout << "Clique is false" << "\n";}

                if (graph_size+2>max_clique_size && clique_found==true){max_clique_size=graph_size+2;}

            }
        }
        //if (i%100==0) {
        cout << "i=" << i << "\n";
        cout << "Currrent largest Clique Size: " << max_clique_size << "\n";
        //}
    }

    //check that all vertices which are in_subgraph actually form a clique

    std::cout << "\n";
    

    //the resulting independent set size is the size of one of the cliques in the intersection graph
    //cout << "We found a valid clique in the graph: " << clique_check << "\n";
    //cout << "Greedy Clique Size: " << graph_size << "\n";

}