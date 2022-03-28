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
#include <map>

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

    ifstream file("instances/rvisp24116.instance.json");
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
    std::vector<int> in_subgraph; in_subgraph.clear();
    std::vector<int> n_edges_among_neighbors; n_edges_among_neighbors.clear();
    //std::vector<std::vector<std::vector<int>>> edges_among_neighbors; edges_among_neighbors.clear();
    std::map<std::pair<int,int>, std::vector<int>> edges_among_neighbors; edges_among_neighbors.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        in_subgraph.push_back(1);
        n_edges_among_neighbors.push_back(0);
    }

    //std::vector<std::vector<std::vector<int>>> edges_among_neighbors(m, vector<vector<int>>(m, vector<int>(m)));

    // for (int i=0; i<m; i++){
    //     cout << "i=" << i << "\n";
    //     edges_among_neighbors.push_back({});
    //     for (int j=0; j<m; j++){
    //         edges_among_neighbors[i].push_back({});
    //         for (int l=j; l<m; l++){
    //             edges_among_neighbors[i][j].push_back(0);
    //         }
    //     }
    // }
    //edges_among_neighbors[i][j][l]==1 iff j and l are negighbors of a and there is an edge between j and l

    for (int i=0; i<m; i++){
        if (i%100==0) {cout << "i=" << i << "\n";}
        for (int j=i; j<m; j++){
            if (adj_matrix[i][j]==1){
                std::vector<int> v_vec; v_vec.clear();
                for (int l=0; l<m; l++){
                    if (adj_matrix[i][l]==1 && adj_matrix[j][l]==1){
                        v_vec.push_back(l);
                        n_edges_among_neighbors[l]++;
                    }
                }
                edges_among_neighbors.insert(std::make_pair(std::make_pair(i,j), v_vec));
            }
        }
    }

    // //determine edges among neighbors for all vertices in intersection graph
    // for (int i=0; i<m; i++){
    //     cout << "i=" << i << "\n";
    //     for (int j=0; j<adjacency_list[i].size(); j++){
    //         for (int l=0; l<adjacency_list[i].size(); l++){
    //             int x=adjacency_list[i][j];
    //             int y=adjacency_list[i][l];
    //             if (adj_matrix[x][y]==1 && x>y) {
    //                 edges_among_neighbors[i][x][y]=1;
    //                 n_edges_among_neighbors[i]++;
    //             } else if (adj_matrix[x][y]==1 && y>x) {
    //                 edges_among_neighbors[i][y][x]=0;
    //             }
    //         }
    //     }
    // }

    int graph_size=m;
    bool clique_found=false;
    while (graph_size>0 && clique_found==false){
        if (graph_size%100==0) {std::cout << "graph_size=" << graph_size << "\n";}
        //find vertex v with smallest number of edges among neighbors
        int min_edges_among_neighbors=m*m;
        int v=0;
        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1 && n_edges_among_neighbors[i]<min_edges_among_neighbors){
                v=i;
                min_edges_among_neighbors=n_edges_among_neighbors[i];
            }
        }

        //if min_edges_among_neighbors==(graph_size-1)(graph_size-2)/2: clique_found=true
        if (min_edges_among_neighbors==(graph_size-1)*(graph_size-2)/2) {
            clique_found=true;
            std::cout << "Found clique of size " << graph_size << "\n";
        } else {
            in_subgraph[v]=0;
            graph_size--;
            //update edges among neighbors
            for (int i=0; i<adjacency_list[v].size(); i++){
                if (in_subgraph[adjacency_list[v][i]]==1){
                    int c_v=adjacency_list[v][i];
                    int x=v;
                    int y=c_v;
                    if (x>y){
                        x=c_v;
                        y=v;
                    }
                    std::vector<int> v_vec=edges_among_neighbors[std::make_pair(x,y)];
                    int len=v_vec.size();
                    for (int j=0; j<len; j++){
                        if (in_subgraph[v_vec[j]]){
                            n_edges_among_neighbors[v_vec[j]]--;
                        }
                    }
                }
            }
        }
    }

    //check whether we can add one of the vertices which is not in the subgraph
    for (int i=0; i<m; i++){
        //cout << i << "\n";
        if (in_subgraph[i]==0){
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

    //check that all vertices which are in_subgraph actually form a clique
    bool clique_check=true;
    for (int i=0; i<m; i++){
        if (in_subgraph[i]==1){
            //cout << "Degree of i: " << degrees[i] << "\n";
            int v_counter=0;
            for (int j=0; j<adjacency_list[i].size(); j++){
                if (in_subgraph[adjacency_list[i][j]]==1) {v_counter++;}
            }
            if (v_counter!=graph_size-1) {clique_check=false;}
        }
    }

    std::cout << "\n";
    

    //the resulting independent set size is the size of one of the cliques in the intersection graph
    cout << "We found a valid clique in the graph: " << clique_check << "\n";
    cout << "Greedy Clique Size: " << graph_size << "\n";

}