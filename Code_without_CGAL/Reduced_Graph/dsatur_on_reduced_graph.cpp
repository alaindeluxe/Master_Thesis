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

    ifstream file("instances/sqrp7730.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int col_bound=300;

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
    std::vector<std::vector<int>> adjacency_list; //adjacency_list_2;
    adjacency_list.clear(); //adjacency_list_2.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        //adjacency_list_2.push_back({});
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
    for (int i=0; i<m; i++){
        edge_colors.push_back(-1);
    }

    //Remove cliques from the remains of the complement intersection graph until we colored all vertices
    int graph_size=m;
    int colors_counter=0;
    int n_v=0;
    int num_v=0;
    std::vector<int> considered_vertices; considered_vertices.clear();
    for (int i=0; i<n; i++) {considered_vertices.push_back(0);}



    cout << "graph size = " << graph_size << "\n";

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

    //check whether we can add one of the vertices which is not in the subgraph
    for (int i=0; i<m; i++){
        //cout << i << "\n";
        if (in_subgraph[i]==0 && considered_vertices[edge_i[i]]==0 && considered_vertices[edge_j[i]]==0){
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

    cout << "Found clique of size " << graph_size << "\n";

    //compute the subgraph induced by the vertices/endpoints of this "edge clique" this subgraph can be colored with c colors.
    //clique edges are inside the array in_subgraph
    for (int i=0; i<m; i++){
        if (in_subgraph[i]==1){
            considered_vertices[edge_i[i]]=1;
            considered_vertices[edge_j[i]]=1;
            n_v=n_v+2;
        }
    }

    //determine the partitions for the edges in this induced subgraph
    for (int i=0; i<m; i++){
        if (in_subgraph[i]==1){
            edge_colors[i]=colors_counter;
            num_v++;
            int a=edge_i[i];
            int b=edge_j[i];
            for (int j=0; j<m; j++){
                if (j!=i && edge_colors[j]==-1){
                    if (edge_i[j]==a && ccw(edge_j[j],a,b)){edge_colors[j]=colors_counter; num_v++;}
                    if (edge_j[j]==a && ccw(edge_i[j],a,b)){edge_colors[j]=colors_counter; num_v++;}
                    if (edge_i[j]==b && ccw(edge_j[j],b,a)){edge_colors[j]=colors_counter; num_v++;}
                    if (edge_j[j]==b && ccw(edge_i[j],b,a)){edge_colors[j]=colors_counter; num_v++;}
                }
            }

            colors_counter++;
        }
    }

    cout << "Colors_Counter: " << colors_counter << "\n";

    //reset remaining graph
    // graph_size=0;
    // for (int i=0; i<m; i++){
    //     if (considered_vertices[edge_i[i]]==0 && considered_vertices[edge_j[i]]==0){
    //         graph_size++;
    //         in_subgraph[i]=1;
    //     } else {
    //         in_subgraph[i]=0;
    //     }
    // }

    // for (int i=0; i<m; i++){
    //     if (in_subgraph[i]==1){
    //         degrees[i]=0;
    //         for (int j=0; j<adjacency_list[i].size(); j++){
    //             if (in_subgraph[adjacency_list[i][j]]==1){
    //                 degrees[i]++;
    //             }
    //         }
    //     }
    // }






    //apply dsatur on the preprocessed coloring

    //update saturation and degrees of vertices
    std::vector<std::vector<int>> neighbors_colors; neighbors_colors.clear();
    for (int i=0; i<m; i++){
        neighbors_colors.push_back({});
        for (int j=0; j<col_bound; j++){
            neighbors_colors[i].push_back(0);
        }
    }

    std::vector<int> saturation; saturation.clear();
    for (int i=0; i<m; i++){
        saturation.push_back(0);
        int c=0;
        for (int j=0; j<adjacency_list[i].size(); j++){
            if (edge_colors[adjacency_list[i][j]]>=0){
                if (neighbors_colors[i][edge_colors[adjacency_list[i][j]]]==0){c++;}
                neighbors_colors[i][edge_colors[adjacency_list[i][j]]]=1;
            }
        }
        saturation[i]=c;
    }

    for (int i=0; i<m; i++){
        degrees[i]=adjacency_list[i].size();
    }



    std::vector<int> used_colors; used_colors.clear();
    std::vector<int> used_colors_c; used_colors_c.clear();
    for (int i=0; i<n; i++){
        used_colors_c.push_back(0);
    }





    //go through the remaining vertices and color them by saturation

    int maxSat_vertex=0;
    int maxSat=0;
    int maxDegree=0;
    for (int i=0; i<m; i++){
        if (saturation[i]>maxSat){
            maxSat_vertex=i;
            maxSat=saturation[i];
            maxDegree=degrees[i];
        } else if (saturation[i]==maxSat && degrees[i]>maxDegree){
            maxSat_vertex=i;
            maxSat=saturation[i];
            maxDegree=degrees[i];
        }
    }

    bool new_color;

    std::cout << "Number of edges in subgraph: " << num_v << "\n";

        while(num_v<m){
        if (num_v%100==0) {std::cout << "num_v=" << num_v << endl;}
        bool new_color=false;
        int current_vertex=maxSat_vertex;
        std::vector<int> possible_colors; possible_colors.clear();
        for (int j=0; j<col_bound; j++) {possible_colors.push_back(0);}
        //std::cout << "y" << endl;
        for (int j=0; j<adjacency_list[current_vertex].size(); j++){
            if (edge_colors[adjacency_list[current_vertex][j]]>=0){
                possible_colors[edge_colors[adjacency_list[current_vertex][j]]]=1;
            }
        }
        int t=0;
        while (t<used_colors.size() && possible_colors[used_colors[t]]==1) {t++;} 
        if (t<used_colors.size()){
            edge_colors[current_vertex]=used_colors[t];
            used_colors_c[used_colors[t]]++;
            for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                if (neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]==0){
                    neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]=1;
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
            if (t>=used_colors.size()-col_bound/2) {new_color=true;}
        } else {
            new_color=true;
            t=0;
            while (possible_colors[t]>0) {t++;}
            edge_colors[current_vertex]=t;
            used_colors_c[t]++;
            used_colors.push_back(t);
            colors_counter++;
            for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                if (neighbors_colors[adjacency_list[current_vertex][j]][t]==0){
                    neighbors_colors[adjacency_list[current_vertex][j]][t]=1;
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
        }

        //sort color classes such that the biggest color classes have small color
        // std::vector<int> color_class_size; color_class_size.clear();
        // for (int i=0; i<col_bound; i++){
        //     color_class_size.push_back(0);
        // }

        // for (int i=0; i<m; i++){
        //     if (edge_colors[i]>=0){
        //         color_class_size[edge_colors[i]]++;
        //     }
        // }

        // std::vector<std::pair<int,int>> colors_with_size; colors_with_size.clear();

        // for (int i=0; i<col_bound; i++){
        //     colors_with_size.push_back(std::make_pair(i, color_class_size[i]));
        // }

        // sort(colors_with_size.begin(),colors_with_size.end(), myComp_high_to_low);

        // std::vector<int> color_mapping; color_mapping.clear();
        // for (int i=0; i<col_bound; i++){
        //     color_mapping.push_back(0);
        // }

        // for (int i=0; i<col_bound; i++){
        //     color_mapping[colors_with_size[i].first]=i;
        // }

        // for (int i=0; i<m; i++){
        //     if (edge_colors[i]>=0){
        //         edge_colors[i]=color_mapping[edge_colors[i]];
        //     }
        // }

        //cout << "s" << "\n";







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

        num_v++;
    }

    colors_counter=0;
    for (int i=0; i<m; i++){
        if (edge_colors[i]+1>colors_counter){colors_counter=edge_colors[i]+1;}
    }



















    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int j=0; j<m; j++){
        for (int l=0; l<adjacency_list[j].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[j]==edge_colors[adjacency_list[j][l]]){
                    valid_color_assignment=false;
                    cout << "edge " << j << " and edge " << adjacency_list[j][l] << " have the same color, but shouldn't" << "\n";
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

     ofstream o("dsatur_impr_on_reduced_graph_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in dsatur_impr_on_reduced_graph Algorithm: " << colors_counter << "\n";

}