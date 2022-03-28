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

    ifstream file("dimacs_instances/C4000.5.col.txt");
    //dsjc1000.5: 101 colors (a=3, b=num_v/5)
    //dsjc1000.1: 24 colors (a=3, b=num_v/5)
    //dsjc1000.9: 
    //dsjc500.5: 59 colors
    //dsjc250.5: 33 colors
    //flat300_28_0: 37 colors
    //le450_25c: 28 colors
    //flat1000_76_0: 101 colors (a=3, b=num_v/5)

    //C2000.5: Found clique of size 9
    //C4000.5: 12

    int n = 4000;
    int m = 4000;
    int col_bound=n;

    //perform algorithm
    //compute intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list, adj_matrix;
    adjacency_list.clear(); adj_matrix.clear();
    for (int l=0; l<n; l++){
        adjacency_list.push_back({});
    }
    for (int i=0; i<m; i++){
        adj_matrix.push_back({});
        for (int j=0; j<m; j++){
            adj_matrix[i].push_back(0);
        }
    }

    //read file line by line

    std::string str;
    //std::stringstream ss;
    while (getline(file,str)){
    //while(std::cin >> str){
        //cout << str << "\n";
        std::stringstream ss;
        ss << str;
        int b, c;
        char a;
        ss >> a >> b >> c;
        //cin >> a >> b >> c;
        //cout << a << " " << b << " " << c << "\n";
        //cout << b << " " << c << "\n";
        adjacency_list[b-1].push_back(c-1);
        adjacency_list[c-1].push_back(b-1);
        adj_matrix[b-1][c-1]=1;
        adj_matrix[c-1][b-1]=1;
        //m++;
    }

    //determine degrees in complement of intersection graph
    //std::vector<std::pair<int,int>> degree_complement_intersection_graph; degree_complement_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    std::vector<int> in_subgraph; in_subgraph.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
        degrees.push_back(temp);
        in_subgraph.push_back(1);
        //degree_complement_intersection_graph.push_back(std::make_pair(l,temp));
    }

    int graph_size=m;
    bool clique_found=false;
    while (graph_size>0 && clique_found==false){
        //find vertex v with smallest degree in subgraph
        int min_degree=m;
        int v=0;
        for (int i=0; i<m; i++){
            if (in_subgraph[i]==1 && degrees[i]<min_degree){
                v=i;
                min_degree=degrees[i];
            }
        }

        //if v has degree graph_size-1: clique_found=true
        if (min_degree==graph_size-1) {
            clique_found=true;
            std::cout << "Found clique of size " << graph_size << "\n";
        } else {
            in_subgraph[v]=0;
            graph_size--;
            for (int i=0; i<adjacency_list[v].size(); i++){
                if (in_subgraph[adjacency_list[v][i]]==1){
                    degrees[adjacency_list[v][i]]--;
                }
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
    cout << "We found a valid clique: " << clique_check << "\n";
    cout << "Greedy Clique Size in graph: " << graph_size << "\n";

}