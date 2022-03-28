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

int main(){
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    ifstream file("instances/reecn3382.instance.json");
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

    cout << "Computed Intersection Graph" << "\n";

    //compute the matrix S[l][k]
    //k<=l
    std::vector<std::vector<int>> s; s.clear();
    for (int l=0; l<m; l++){
        s.push_back({});
        for (int k=0; k<=l; k++){
            //compute S_lk
            if (k==l || intersection_graph[l][k]==1){
                s[l].push_back(0);
            } else {
                //copmute how many vertices are adjacent to both k and l
                int s_count=0;
                for (int j=0; j<m; j++){
                    if (intersection_graph[k][j]==1 && intersection_graph[l][j]==1) {s_count++;}
                }
                s[l].push_back(s_count);
            }
            cout << "Computed S for " << l << " and " << k << "\n";
        }
    }

    cout << "Computed Matrix S" << "\n";

    //sort vertex pairs from highest similarity to lowest
    //(k<=l)
    std::vector<std::tuple<int,int,int>> vertex_pairs_with_similarity; vertex_pairs_with_similarity.clear();
    for (int l=0; l<m; l++){
        for (int k=0; k<=l; k++){
            vertex_pairs_with_similarity.push_back(std::make_tuple(l,k,s[l][k]));
        }
    }
    sort(vertex_pairs_with_similarity.begin(), vertex_pairs_with_similarity.end(), myComparison3);


    //for (int l=0; l<vertex_pairs_with_similarity.size(); l++){
    //    cout << std::get<0>(vertex_pairs_with_similarity[l]) << " " <<  std::get<1>(vertex_pairs_with_similarity[l]) << " " <<  std::get<2>(vertex_pairs_with_similarity[l]) << "\n";
    //}

    //determine degrees in intersection graph
    std::vector<int> degrees; degrees.clear();
    std::vector<std::pair<int,int>> degree_intersection_graph; degree_intersection_graph.clear();
    for (int l=0; l<m; l++){
        int temp=0;
        for (int k=0; k<m; k++){
            if (intersection_graph[l][k]==1) {temp++;}
        }
        degrees.push_back(temp);
        degree_intersection_graph.push_back(std::make_pair(l,temp));
    }

    sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);
    cout << "Computed Degrees in Intersection Graph" << "\n";


    //go over edge pairs and color them according to the coloring pairs algorithm
    std::vector<int> edge_colors; edge_colors.clear();
    std::vector<int> used_colors; used_colors.clear(); //contains all the colors that we already used
    int colors_counter=0;
    for (int k=0; k<m; k++){
        edge_colors.push_back(-1);
    }

    for (int i=0; i<vertex_pairs_with_similarity.size(); i++){

        int vertex_1=std::get<0>(vertex_pairs_with_similarity[i]);
        int vertex_2=std::get<1>(vertex_pairs_with_similarity[i]);

        if (edge_colors[vertex_1]>=0 && edge_colors[vertex_2]>=0){
            //go to next pair
        } else if ((edge_colors[vertex_1]==-1 && edge_colors[vertex_2]>=0) || (edge_colors[vertex_1]>=0 && edge_colors[vertex_2]==-1)){
            //vi is colored with color r
            int vi, vj;
            if (edge_colors[vertex_1]>=0){
                vi=vertex_1;
                vj=vertex_2;
            } else {
                vi=vertex_2;
                vj=vertex_1;
            }
            //If the degree of Vj is less than the number of colours already used, proceed to the next pair.
            if (degrees[vj]<colors_counter){
                //go to next pair
            } else {//If Vj is adjacent to a vertex already coloured r, go to next pair, else color it with r
                int check=0;
                for (int k=0; k<m; k++){
                    if (intersection_graph[k][vj]==1 && edge_colors[k]==edge_colors[vi]) {check=1;}
                }
                if (check==0){
                    edge_colors[vj]=edge_colors[vi];
                }
            }
             
        } else {

            int vi=vertex_1;
            int vj=vertex_2;

            //If the degree of both vt and v} is less than the number of colours already used, proceed to the next pair
            if (degrees[vi]<colors_counter && degrees[vj]<colors_counter){
                //go to next pair
            }
            //Colour vt and Vj with the first already used colour which has no vertices adjacent to vt and Vj.
            std::vector<int> possible_colors; possible_colors.clear();
            for (int k=0; k<m; k++){
                possible_colors.push_back(0);//0 means available, 1 means taken
            }
            for (int k=0; k<m; k++){
                if (intersection_graph[k][vi]==1 && edge_colors[k]>=0){
                    possible_colors[edge_colors[k]]=1;
                }
                if (intersection_graph[k][vj]==1 && edge_colors[k]>=0){
                    possible_colors[edge_colors[k]]=1;
                }
            }

            int used_colors_index=0;
            int check=0;
            while (used_colors_index<used_colors.size() && check==0){
                if (possible_colors[used_colors[used_colors_index]]==0){
                    check=1;// we have found a possible color that is in used_colors
                } else {
                    used_colors_index++;
                }
            }

            if (check==1){
                edge_colors[vi]=used_colors[used_colors_index];
                edge_colors[vj]=used_colors[used_colors_index];
            } else {//If vt and Vj cannot both be coloured with a previously used colour, they become the first vertices with new colours.
                int t=0;
                while (possible_colors[t]==1) {t++;}
                edge_colors[vi]=t;
                edge_colors[vj]=t;
                used_colors.push_back(t);
                colors_counter++;
            }
        }
            
    }


    //color omitted vertices with an already used color if possible

    for (int j=0; j<m; j++){
        int i=degree_intersection_graph[j].first;
        if (edge_colors[i]==-1){
            //color it with a used color if possible
            std::vector<int> possible_colors; possible_colors.clear();
            for (int k=0; k<m; k++){
                possible_colors.push_back(0);//0 means available, 1 means taken
            }
            for (int k=0; k<m; k++){
                if (intersection_graph[k][i]==1 && edge_colors[k]>=0){
                    possible_colors[edge_colors[k]]=1;
                }
            }

            int used_colors_index=0;
            int check=0;

            while (used_colors_index<used_colors.size() && check==0){
                if (possible_colors[used_colors[used_colors_index]]==0){
                    check=1;// we have found a possible color that is in used_colors
                } else {
                    used_colors_index++;
                }
            }

            if (check==1){
                edge_colors[i]=used_colors[used_colors_index];
            } else {
                int t=0;
                while (possible_colors[t]==1) {t++;}
                edge_colors[i]=t;
                used_colors.push_back(t);
                colors_counter++;
            }
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

     ofstream o("coloring_pairs_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in Coloring Pairs Algorithm: " << colors_counter << "\n";

}