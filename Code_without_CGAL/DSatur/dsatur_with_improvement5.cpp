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

bool myComp_low_to_high(const pair<int,int> &a,const pair<int,int> &b)
{
       return a.second<b.second;
}

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/rvisp5013.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int col_bound=70;

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

    //determine degrees in intersection graph
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    std::vector<int> degrees; degrees.clear();
    for (int l=0; l<m; l++){
        int temp=adjacency_list[l].size();
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
        for (int j=0; j<col_bound; j++){
            neighbors_colors[i].push_back(0);
        }
    }

    int maxSat=0;
    int maxDegree=0;
    int maxSat_vertex=degree_intersection_graph[0].first;
    int num_v=1;

    int colors_counter=0;
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> used_colors; used_colors.clear();//contains all colors that have already been assigned
    for (int k=0; k<m; k++){
        edge_colors.push_back(-1);
    }
    std::vector<int> used_colors_c; used_colors_c.clear();
    for (int i=0; i<n; i++){
        used_colors_c.push_back(0);
    }

    while(num_v<=m){
        std::cout << "num_v=" << num_v << endl;
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
                //new in impr5
                neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]++;
                if (neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]==1){
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
        } else {
            t=0;
            while (possible_colors[t]>0) {t++;}
            edge_colors[current_vertex]=t;
            used_colors_c[t]++;
            used_colors.push_back(t);
            colors_counter++;
            for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                //new in impr5
                neighbors_colors[adjacency_list[current_vertex][j]][t]++;
                if (neighbors_colors[adjacency_list[current_vertex][j]][t]==1){
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
        }

        //sort color classes such that the biggest color classes have small color
        std::vector<int> color_class_size; color_class_size.clear();
        for (int i=0; i<col_bound; i++){
            color_class_size.push_back(0);
        }

        for (int i=0; i<m; i++){
            if (edge_colors[i]>=0){
                color_class_size[edge_colors[i]]++;
            }
        }

        std::vector<std::pair<int,int>> colors_with_size; colors_with_size.clear();

        for (int i=0; i<col_bound; i++){
            colors_with_size.push_back(std::make_pair(i, color_class_size[i]));
        }

        sort(colors_with_size.begin(),colors_with_size.end(), myComp_high_to_low);

        std::vector<int> color_mapping; color_mapping.clear();
        for (int i=0; i<col_bound; i++){
            color_mapping.push_back(0);
        }

        for (int i=0; i<col_bound; i++){
            color_mapping[colors_with_size[i].first]=i;
        }

        for (int i=0; i<m; i++){
            if (edge_colors[i]>=0){
                edge_colors[i]=color_mapping[edge_colors[i]];
            }
        }

        //cout << "s" << "\n";

        //update neighbors_colors, saturation doesn't change
        std::vector<std::vector<int>> neighbors_colors_temp; neighbors_colors_temp.clear();
        for (int i=0; i<m; i++){
            neighbors_colors_temp.push_back({});
            for (int j=0; j<col_bound; j++){
                neighbors_colors_temp[i].push_back(0);
            }
        }

        for (int i=0; i<m; i++){
            for (int j=0; j<col_bound; j++){
                neighbors_colors_temp[i][color_mapping[j]]=neighbors_colors[i][j];
            }
        }

        neighbors_colors=neighbors_colors_temp;























        //use improvement algorithm

        //sort vertices of intersection graph by their assigned color, from highest to lowest

        for (int a=0; a<1; a++){

            //if (num_v>=800) {std::cout << "a=" << a << "\n";}

            std::vector<std::pair<int,int>> vertex_with_color; vertex_with_color.clear();

            for (int i=0; i<m; i++){
                vertex_with_color.push_back(std::make_pair(i,edge_colors[i]));
            }

            sort(vertex_with_color.begin(),vertex_with_color.end(), myComp_high_to_low);

            //we now go through the list vertex_with_color and for each vertex, we try to assign it a smaller color

            for (int ii=0; ii<min(3,num_v); ii++){//b=m
                //if (num_v>=100) {std::cout << "ii = " << ii << "\n";}
                int current_v=vertex_with_color[ii].first;
                //find neighbors of vertex current_v and sort them by color
                std::vector<std::pair<int,int>> current_neighbors_with_color; current_neighbors_with_color.clear();
                for (int j=0; j<adjacency_list[current_v].size(); j++){
                    //if (i!=j && intersection_graph[i][j]==1){
                    current_neighbors_with_color.push_back(std::make_pair(adjacency_list[current_v][j], edge_colors[adjacency_list[current_v][j]]));
                //}
                }
                sort(current_neighbors_with_color.begin(), current_neighbors_with_color.end(), myComp_high_to_low);//high to low in first version

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
                    if (current_color_works && current_color>=0){
                    //cout << "current color works" << "\n";
        
                    //check whether we can change the colors from first_neighbor to last_neighbor (that have the same color) to a different color between 0 and edge_colors[i].
                    std::vector<int> best_colors; best_colors.clear();//best_colors[i] shows the best possible new color assignment for i
                    for (int k=0; k<=last_neighbor_index-first_neighbor_index; k++){
                        best_colors.push_back(-1);
                    }
                    for (int k=first_neighbor_index; k<=last_neighbor_index; k++){
                        //cout << "k=" << k << "\n";
                        //find possible colors for this vertex that are smaller than edge_colors[i]
                        std::vector<int> possible_colors; possible_colors.clear();
                        for (int l=0; l<n; l++){
                            possible_colors.push_back(0); //possible_colors[i]==1 means that color i is not an option
                        }
                        //cout << "Checking neighbor " << k << "\n";
                        int current_vertex = current_neighbors_with_color[k].first;
                        for (int l=0; l<adjacency_list[current_vertex].size(); l++){
                            if (edge_colors[adjacency_list[current_vertex][l]]>=0){
                                possible_colors[edge_colors[adjacency_list[current_vertex][l]]]=1;
                            }
                        }
                        int t=0;
                        while (possible_colors[t]>0 || t==current_color) {t++;}
                        if (t<edge_colors[current_v]) {best_colors[k-first_neighbor_index]=t;}
                    }

                    //if every vertex has a best_color, we can change the colors of the vertices

                    bool everybody_best_color=true;
                    for (int k=0; k<=last_neighbor_index-first_neighbor_index; k++){
                        if (best_colors[k]==-1) {everybody_best_color=false;}
                    }

                    if (everybody_best_color){
                        std::cout << "Updating colors for ii=" << ii << ", k between " << first_neighbor_index << " " << last_neighbor_index << endl;
                        //update color of vertex i
                        used_colors_c[edge_colors[current_v]]--;
                        if (used_colors_c[edge_colors[current_v]]==0){
                            colors_counter--;
                        }
                        //new in impr5
                        for (int h=0; h<adjacency_list[current_v].size(); h++){
                            neighbors_colors[adjacency_list[current_v][h]][edge_colors[current_v]]--;
                            if (neighbors_colors[adjacency_list[current_v][h]][edge_colors[current_v]]==0){
                                saturation[adjacency_list[current_v][h]]--;
                            }
                        }
                        edge_colors[current_v]=current_color;
                        used_colors_c[current_color]++;
                        if (used_colors_c[current_color]==1){
                            colors_counter++;
                        }
                        //new in impr5
                        for (int h=0; h<adjacency_list[current_v].size(); h++){
                            neighbors_colors[adjacency_list[current_v][h]][current_color]++;
                            if (neighbors_colors[adjacency_list[current_v][h]][current_color]==1){
                                saturation[adjacency_list[current_v][h]]++;
                            }
                        }
                        //update color of current_neighbors
                        for (int k=first_neighbor_index; k<=last_neighbor_index; k++){
                            int current_vertex = current_neighbors_with_color[k].first;
                            used_colors_c[edge_colors[current_vertex]]--;
                            if (used_colors_c[edge_colors[current_vertex]]==0){
                                colors_counter--;
                            }
                            //new in impr5
                            for (int h=0; h<adjacency_list[current_vertex].size(); h++){
                                neighbors_colors[adjacency_list[current_vertex][h]][edge_colors[current_vertex]]--;
                                if (neighbors_colors[adjacency_list[current_vertex][h]][edge_colors[current_vertex]]==0){
                                    saturation[adjacency_list[current_vertex][h]]--;
                                }
                            }
                            edge_colors[current_vertex]=best_colors[k-first_neighbor_index];
                            used_colors_c[edge_colors[current_vertex]]++;
                            if (used_colors_c[edge_colors[current_vertex]]==1){
                                colors_counter++;
                            }
                            for (int h=0; h<adjacency_list[current_vertex].size(); h++){
                                neighbors_colors[adjacency_list[current_vertex][h]][edge_colors[current_vertex]]++;
                                if (neighbors_colors[adjacency_list[current_vertex][h]][edge_colors[current_vertex]]==1){
                                    saturation[adjacency_list[current_vertex][h]]++;
                                }
                            }
                        }
                    }

                    }
                //cout << "move on" << "\n";

                //else we move on

                first_neighbor_index=last_neighbor_index+1;
                last_neighbor_index=first_neighbor_index;

                }
    
            }

            //make sure every vertex has lowest possible color
            for (int ii=0; ii<num_v-1; ii++){
                int current_vertex=vertex_with_color[ii].first;
                std::vector<int> possible_colors; possible_colors.clear();
                for (int j=0; j<col_bound; j++) {possible_colors.push_back(0);}
                for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                    if (edge_colors[adjacency_list[current_vertex][j]]>=0){
                        possible_colors[edge_colors[adjacency_list[current_vertex][j]]]=1;
                    }
                }
                int t=0;
                while (possible_colors[t]==1) {t++;}

                used_colors_c[edge_colors[current_vertex]]--;
                if (used_colors_c[edge_colors[current_vertex]]==0){
                    colors_counter--;
                }
                edge_colors[current_vertex]=t;
                used_colors_c[t]++;
                if (used_colors_c[t]==1){
                    colors_counter++;
                }

            }

            //order color classes

            color_class_size.clear();
            for (int i=0; i<col_bound; i++){
                color_class_size.push_back(0);
            }

            for (int i=0; i<m; i++){
                if (edge_colors[i]>=0){
                    color_class_size[edge_colors[i]]++;
                }
            }

            colors_with_size.clear();

            for (int i=0; i<col_bound; i++){
                colors_with_size.push_back(std::make_pair(i, color_class_size[i]));
            }

            sort(colors_with_size.begin(),colors_with_size.end(), myComp_high_to_low);

            color_mapping.clear();
            for (int i=0; i<col_bound; i++){
                color_mapping.push_back(0);
            }

            for (int i=0; i<col_bound; i++){
                color_mapping[colors_with_size[i].first]=i;
            }

            for (int i=0; i<m; i++){
                if (edge_colors[i]>=0){
                    edge_colors[i]=color_mapping[edge_colors[i]];
                }
            }

            //update neighbors_colors, saturation doesn't change
            std::vector<std::vector<int>> neighbors_colors_temp; neighbors_colors_temp.clear();
            for (int i=0; i<m; i++){
                neighbors_colors_temp.push_back({});
                for (int j=0; j<col_bound; j++){
                    neighbors_colors_temp[i].push_back(0);
                }
            }

            for (int i=0; i<m; i++){
                for (int j=0; j<col_bound; j++){
                    neighbors_colors_temp[i][color_mapping[j]]=neighbors_colors[i][j];
                }
            }

            neighbors_colors=neighbors_colors_temp;

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

        num_v++;
    }

    //check number of colors
    int max_c=0;
    for (int i=0; i<m; i++){
        if (edge_colors[i]>max_c) {max_c=edge_colors[i];}
    }

    colors_counter=max_c+1;
    

    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int k=0; k<m; k++){
        if (edge_colors[k]==-1) {valid_color_assignment=false;}
        for (int l=0; l<adjacency_list[k].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[k]==edge_colors[adjacency_list[k][l]]){
                    valid_color_assignment=false;
                    std::cout << "edge " << k << " and edge " << adjacency_list[k][l] << " have the same color: " << edge_colors[k] <<  "\n";
                }
            //}
        }
    }

    std::cout << "Color Assignment is valid: " << valid_color_assignment << "\n";

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

     ofstream o("dsatur_impr5_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     std::cout << "Number of Colors in DSatur w.Impr.5 Algorithm: " << colors_counter << "\n";

}