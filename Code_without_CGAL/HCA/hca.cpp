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

    ifstream file("instances/vispecn2518.instance.json");
    Json::Value graphJson;
    Json::Reader reader;
    reader.parse( file, graphJson);

    int n = graphJson["n"].asInt();
    int m = graphJson["m"].asInt();
    int k=65;
    int p=10;
    int L=250;
    int A=10;
    int alpha=0.9;

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


    //sort edges from highest degree to lowest degree
    //sort(degree_intersection_graph.begin(),degree_intersection_graph.end(), myComparison);

    //start by initializing p populations/configurations with k classes

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

    while(i<m && colors_counter<=k){
        int current_vertex=maxSat_vertex;
        std::vector<int> possible_colors; possible_colors.clear();
        for (int j=0; j<k; j++) {possible_colors.push_back(0);}
        for (int j=0; j<adjacency_list[current_vertex].size(); j++){
            if (edge_colors[adjacency_list[current_vertex][j]]>=0){
                possible_colors[edge_colors[adjacency_list[current_vertex][j]]]=1;
            }
        }
        int t=0;
        while (t<used_colors.size() && possible_colors[used_colors[t]]==1) {t++;} 
        if (t<used_colors.size()){
            edge_colors[current_vertex]=used_colors[t];
            for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                if (neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]==0){
                    neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]=1;
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
        } else {
            t=0;
            while (possible_colors[t]>0) {t++;}
            edge_colors[current_vertex]=t;
            used_colors.push_back(t);
            colors_counter++;
            for (int j=0; j<adjacency_list[current_vertex].size(); j++){
                if (neighbors_colors[adjacency_list[current_vertex][j]][t]==0){
                    neighbors_colors[adjacency_list[current_vertex][j]][t]=1;
                    saturation[adjacency_list[current_vertex][j]]++;
                }
            }
        }

        //find vertex with highest saturation
        maxSat=0; maxDegree=0;
        for (int j=0; j<m; j++){
            if (edge_colors[j]==-1){
                if (saturation[j]>maxSat && saturation[j]<=k-1){
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

    //saturation.clear(); used_colors.clear(); neighbors_colors.clear();

    //the best configuration so far will be stored in edge_colors


    //some vertices may be uncolored
    //we assign these vertices random colors in each population
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,k-1);
    std::uniform_int_distribution<std::mt19937::result_type> dist_pop(0,p-1);
    std::uniform_int_distribution<std::mt19937::result_type> distA(0,A-1);

    std::vector<std::vector<int>> populations; populations.clear();
    std::vector<int> bad_crossings; bad_crossings.clear();
    int best_bad_crossings=m*(m-1)/2;
    std::vector<int> best_configuration, current_configuration;
    for (int i=0; i<p; i++){
        //cout << "Initializing population " << i << "\n";
        populations.push_back({});
        for (int j=0; j<m; j++){
            //cout << "j=" << j << "\n";
            if (edge_colors[j]>=0){
                populations[i].push_back(edge_colors[j]);
            } else {
                int x = dist(rng);
                populations[i].push_back(x);
            }

        }

        //determine bad_crossings
        int bc=0;
        for (int j=0; j<m; j++){
                for (int l=0; l<adjacency_list[j].size(); l++){
                    if (populations[i][j]==populations[i][adjacency_list[j][l]]){
                        bc++;
                    }
                }
        }

        bc=bc/2;
        bad_crossings.push_back(bc);

        // improve population i by doing local search on it for L iterations
        std::vector<std::vector<int>> tabu_list; tabu_list.clear();
        for (int j=0; j<m; j++){
            //cout << "tabulist j=" << j << "\n";
            tabu_list.push_back({});
            for (int l=0; l<k; l++){
                tabu_list[j].push_back(0);
            }
        }

        std::vector<std::vector<int>> possible_moves; possible_moves.clear();
        for (int j=0; j<m; j++){
                possible_moves.push_back({});
                for (int l=0; l<k; l++){
                    possible_moves[j].push_back(0);
                }
        }

        int it=0;

        //initialize possible_moves
        // for (int j=0; j<m; j++){
        //     for (int l=0; l<k; l++){
        //         possible_moves[j][l]=0;
        //     }
        // }

        int current_bad_crossings=bad_crossings[i];
        current_configuration=populations[i];

        //calculate possible moves
        for (int j=0; j<m; j++){
            for (int l=0; l<adjacency_list[j].size(); l++){
                if (populations[i][j]!=populations[i][adjacency_list[j][l]]){
                    possible_moves[j][populations[i][adjacency_list[j][l]]]--;
                    possible_moves[adjacency_list[j][l]][populations[i][j]]--;
                } else {
                    int c=populations[i][j];
                    for (int o=0; o<c; o++){
                        possible_moves[j][o]++;
                        possible_moves[adjacency_list[j][l]][o]++;
                    }
                    for (int o=c+1; o<k; o++){
                        possible_moves[j][o]++;
                        possible_moves[adjacency_list[j][l]][o]++;
                    }
                }
            }
        }

        while (it<L){

            //cout << "it= " <<  it << "\n";
            //initialize possible_moves
            // for (int j=0; j<m; j++){
            //     for (int l=0; l<k; l++){
            //         possible_moves[j][l]=0;
            //     }
            // }

            //calculate best possible authorized move
            // for (int j=0; j<m; j++){
            //     for (int l=0; l<adjacency_list[j].size(); l++){
            //         if (populations[i][j]!=populations[i][adjacency_list[j][l]]){
            //             possible_moves[j][populations[i][adjacency_list[j][l]]]--;
            //             possible_moves[adjacency_list[j][l]][populations[i][j]]--;
            //         } else {
            //             int c=populations[i][j];
            //             for (int o=0; o<c; o++){
            //                 possible_moves[j][o]++;
            //                 possible_moves[adjacency_list[j][l]][o]++;
            //             }
            //             for (int o=c+1; o<k; o++){
            //                 possible_moves[j][o]++;
            //                 possible_moves[adjacency_list[j][l]][o]++;
            //             }
            //         }
            //     }
            // }

            int best_j, best_l;
            int best_value=-m;

            //find best possible authorized move
            for (int j=0; j<m; j++){
                for (int l=0; l<k; l++){
                    if ((tabu_list[j][l]==0 && possible_moves[j][l]>best_value && populations[i][j]!=l) || (possible_moves[j][l]>0 && possible_moves[j][l]>best_value && populations[i][j]!=l)){
                        best_j=j; best_l=l;
                        best_value=possible_moves[j][l]/2;
                    }
                }
            }

            int temp = distA(rng);
            int tl=temp+alpha*bad_crossings[i];

            tabu_list[best_j][populations[i][best_j]]=tl+1;
            for (int j=0; j<m; j++){
                for (int l=0; l<k; l++){
                    tabu_list[j][l]=std::max(0,tabu_list[j][l]-1);
                }
            }

            int x=populations[i][best_j];
            int y=best_l;

            for (int l=0; l<adjacency_list[best_j].size(); l++){
                if (populations[i][adjacency_list[best_j][l]]==y){
                    //possible_moves[adjacency_list[best_j][l]][y]=0;
                    for (int o=0; o<k; o++){
                        if (o!=y && o==x) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]+4;}
                        else if (o!=y) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]+2;}
                    }
                } else if (populations[i][adjacency_list[best_j][l]]==x){
                    //possible_moves[adjacency_list[best_j][l]][x]=0;
                    for (int o=0; o<k; o++){
                        if (o!=x && o==y) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]-4;}
                        else if (o!=x) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]-2;}
                    }
                } else {
                    possible_moves[adjacency_list[best_j][l]][x]=possible_moves[adjacency_list[best_j][l]][x]+2;
                    possible_moves[adjacency_list[best_j][l]][y]=possible_moves[adjacency_list[best_j][l]][y]-2;
                }
            }

            for (int l=0; l<k; l++){
                possible_moves[best_j][l]=possible_moves[best_j][l]-2*best_value;
            }

            populations[i][best_j]=best_l;
            bad_crossings[i]=bad_crossings[i]-best_value;

            if (bad_crossings[i]<best_bad_crossings){
                best_bad_crossings=bad_crossings[i];
                best_configuration=populations[i];
            }

            if (bad_crossings[i]<current_bad_crossings){
                current_bad_crossings=bad_crossings[i];
                current_configuration=populations[i];
            }

            if (it==L-1) {cout << "Bad crossings: " << best_bad_crossings << "\n";}


            it++;

        }

        populations[i]=current_configuration;
        bad_crossings[i]=current_bad_crossings;
    }


    //determine class sizes
    std::vector<std::vector<int>> class_size; class_size.clear();
    for (int i=0; i<p; i++){
        class_size.push_back({});
        for (int j=0; j<k; j++){
            class_size[i].push_back(0);
        }
    }

    for (int i=0; i<p; i++){
        for (int j=0; j<m; j++){
            class_size[i][populations[i][j]]++;
        }
    }






























    int niter=0;

    while (niter<250){

        //cout << "niter = " << niter << "\n";

        //choose two parent populations randomly
        int pop1 = dist_pop(rng);
        int pop2 = dist_pop(rng);
        if (pop1==pop2) {pop2=(pop2+1)%p;}
        //cout << pop1 << " " << pop2 << "\n";

        //determine which of the parents is worse and keep the other one
        // int pop1_mistakes=0;
        // int pop2_mistakes=0;
        // for (int i=0; i<m; i++){
        //     for (int l=0; l<adjacency_list[i].size(); l++){
        //             if (populations[pop1][i]==populations[pop1][adjacency_list[i][l]]){
        //                 pop1_mistakes++;
        //             }
        //             if (populations[pop2][i]==populations[pop2][adjacency_list[i][l]]){
        //                 pop2_mistakes++;
        //             }
        //     }
        // }

        std::vector<int> keep_population, keep_class_size;
        int bad_crossings_keep;

        if (bad_crossings[pop1]>bad_crossings[pop2]){
            //keep pop2
            keep_population=populations[pop2];
            keep_class_size=class_size[pop2];
            bad_crossings_keep=bad_crossings[pop2];
        } else {
            //keep pop1
            keep_population=populations[pop1];
            keep_class_size=class_size[pop1];
            bad_crossings_keep=bad_crossings[pop1];
        }

        //build new population
        std::vector<int> new_population; new_population.clear();
        for (int i=0; i<m; i++){
            new_population.push_back(-1);
        }

        //do GPX crossover operation
        for (int l=0; l<k; l++){
            int a=pop2;
            if (l%2==0) {a=pop1;}

            //choose largest possible color class in pop1/pop2
            int largest_class_size=0;
            int largest_class=0;
            for (int i=0; i<k; i++){
                if (class_size[a][i]>largest_class_size){
                    largest_class_size=class_size[a][i];
                    largest_class=i;
                }

            }

            //take vertices from largest color class into new_population
            for (int i=0; i<m; i++){
                if (populations[a][i]==largest_class && new_population[i]==-1){
                    new_population[i]=l;
                    class_size[pop1][populations[pop1][i]]--;
                    populations[pop1][i]=-1;
                    class_size[pop2][populations[pop2][i]]--;
                    populations[pop2][i]=-1;
                }
            }
        }

        //some vertices in new_population may be uncolored.
        //we assign random colors to these vertices
        for (int i=0; i<m; i++){
            if (new_population[i]==-1){
                int x=dist(rng);
                new_population[i]=x;
            }
        }

        //determine bad_crossings in new_population
        int bad_crossings_new=0;
        for (int j=0; j<m; j++){
                for (int l=0; l<adjacency_list[j].size(); l++){
                    if (new_population[j]==new_population[adjacency_list[j][l]]){
                        bad_crossings_new++;
                    }
                }
        }

        bad_crossings_new=bad_crossings_new/2;



































        //do local improvement on new_population
        int it=0;
        std::vector<std::vector<int>> tabu_list; tabu_list.clear();
        for (int j=0; j<m; j++){
            tabu_list.push_back({});
            for (int l=0; l<k; l++){
                tabu_list[j].push_back(0);
            }
        }

        std::vector<std::vector<int>> possible_moves; possible_moves.clear();
        for (int j=0; j<m; j++){
            possible_moves.push_back({});
            for (int l=0; l<k; l++){
                possible_moves[j].push_back(0);
            }
        }

        for (int j=0; j<m; j++){
                for (int l=0; l<adjacency_list[j].size(); l++){
                    if (new_population[j]!=new_population[adjacency_list[j][l]]){
                        possible_moves[j][new_population[adjacency_list[j][l]]]--;
                        possible_moves[adjacency_list[j][l]][new_population[j]]--;
                    } else {
                        int c=new_population[j];
                        for (int o=0; o<c; o++){
                            possible_moves[j][o]++;
                            possible_moves[adjacency_list[j][l]][o]++;
                        }
                        for (int o=c+1; o<k; o++){
                            possible_moves[j][o]++;
                            possible_moves[adjacency_list[j][l]][o]++;
                        }
                    }
                }
            }

        current_configuration=new_population;
        int current_bad_crossings=bad_crossings_new;

        while (it<L){

            //cout << "Loc.Impr. of new_pop, it = " << it << "\n";
            //initialize possible_moves
            // for (int j=0; j<m; j++){
            //     for (int l=0; l<k; l++){
            //         possible_moves[j][l]=0;
            //     }
            // }

            //calculate best possible authorized move
            // for (int j=0; j<m; j++){
            //     for (int l=0; l<adjacency_list[j].size(); l++){
            //         if (new_population[j]!=new_population[adjacency_list[j][l]]){
            //             possible_moves[j][new_population[adjacency_list[j][l]]]--;
            //             possible_moves[adjacency_list[j][l]][new_population[j]]--;
            //         } else {
            //             int c=new_population[j];
            //             for (int o=0; o<c; o++){
            //                 possible_moves[j][o]++;
            //                 possible_moves[adjacency_list[j][l]][o]++;
            //             }
            //             for (int o=c+1; o<k; o++){
            //                 possible_moves[j][o]++;
            //                 possible_moves[adjacency_list[j][l]][o]++;
            //             }
            //         }
            //     }
            // }

            int best_j, best_l;
            int best_value=-m;

            //find best possible authorized move
            for (int j=0; j<m; j++){
                for (int l=0; l<k; l++){
                    if ((tabu_list[j][l]==0 && possible_moves[j][l]>best_value && new_population[j]!=l) || (possible_moves[j][l]>0 && possible_moves[j][l]>best_value && new_population[j]!=l)){
                        best_j=j; best_l=l;
                        best_value=possible_moves[j][l]/2;
                    }
                }
            }

            // int bad_crossings=0;
            // for (int j=0; j<m; j++){
            //     //int foo=0;
            //     for (int l=0; l<adjacency_list[j].size(); l++){
            //         if (new_population[j]==new_population[adjacency_list[j][l]]){
            //             bad_crossings++;
            //             //foo=1;
            //         }
            //     }
            // }

            // bad_crossings=bad_crossings/2;

            //if (bad_crossings_new==0) {edge_colors=new_population;}

            int temp = distA(rng);
            int tl=temp+alpha*bad_crossings_new;

            tabu_list[best_j][new_population[best_j]]=tl+1;
            for (int j=0; j<m; j++){
                for (int l=0; l<k; l++){
                    tabu_list[j][l]=std::max(0,tabu_list[j][l]-1);
                }
            }

            int x=new_population[best_j];
            int y=best_l;

            for (int l=0; l<adjacency_list[best_j].size(); l++){
                if (new_population[adjacency_list[best_j][l]]==y){
                    //possible_moves[adjacency_list[best_j][l]][y]=0;
                    for (int o=0; o<k; o++){
                        if (o!=y && o==x) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]+4;}
                        else if (o!=y) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]+2;}
                    }
                } else if (new_population[adjacency_list[best_j][l]]==x){
                    //possible_moves[adjacency_list[best_j][l]][x]=0;
                    for (int o=0; o<k; o++){
                        if (o!=x && o==y) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]-4;}
                        else if (o!=x) {possible_moves[adjacency_list[best_j][l]][o]=possible_moves[adjacency_list[best_j][l]][o]-2;}
                    }
                } else {
                    possible_moves[adjacency_list[best_j][l]][x]=possible_moves[adjacency_list[best_j][l]][x]+2;
                    possible_moves[adjacency_list[best_j][l]][y]=possible_moves[adjacency_list[best_j][l]][y]-2;
                }
            }

            for (int l=0; l<k; l++){
                possible_moves[best_j][l]=possible_moves[best_j][l]-2*best_value;
            }

            new_population[best_j]=best_l;
            bad_crossings_new=bad_crossings_new-best_value;

            if (bad_crossings_new<best_bad_crossings){
                best_bad_crossings=bad_crossings_new;
                best_configuration=new_population;
            }

            if (bad_crossings_new<current_bad_crossings){
                current_bad_crossings=bad_crossings_new;
                current_configuration=new_population;
            }

            if (it==L-1) {cout << "Bad crossings: " << best_bad_crossings << "\n";}


            it++;

            // new_population[best_j]=best_l;
            // bad_crossings_new=bad_crossings_new-best_value;

            // if (it==L-1) {cout << "Bad crossings: " << bad_crossings_new << "\n";}


            // it++;
        }

        new_population=current_configuration;
        bad_crossings_new=current_bad_crossings;



        //determine new_class_size
        std::vector<int> new_class_size; new_class_size.clear();
        for (int l=0; l<k; l++){
            new_class_size.push_back(0);
        }

        for (int l=0; l<m; l++){
            new_class_size[new_population[l]]++;
        }










        //update populations
        if (bad_crossings[pop1]>bad_crossings[pop2]){
            populations[pop1]=new_population;
            class_size[pop1]=new_class_size;
            bad_crossings[pop1]=bad_crossings_new;
            populations[pop2]=keep_population;
            class_size[pop2]=keep_class_size;
            bad_crossings[pop2]=bad_crossings_keep;
        } else {
            populations[pop2]=new_population;
            class_size[pop2]=new_class_size;
            bad_crossings[pop2]=bad_crossings_new;
            populations[pop1]=keep_population;
            class_size[pop1]=keep_class_size;
            bad_crossings[pop1]=bad_crossings_keep;
        }


        niter++;

    }











    //pick best population
    int bc=m;
    int best_p=0;
    for (int i=0; i<p; i++){
        if (bad_crossings[i]<bc){
            best_p=i;
        }
    }

    edge_colors=populations[best_p];























































    //check whether we have a correct color assignment
    bool valid_color_assignment=true;
    for (int i=0; i<m; i++){
        if (edge_colors[i]==-1) {valid_color_assignment=false;}
        for (int l=0; l<adjacency_list[i].size(); l++){
            //if (intersection_graph[k][l]==1 && l!=k){
                if (edge_colors[i]==edge_colors[adjacency_list[i][l]]){
                    valid_color_assignment=false;
                    cout << "edge " << i << " and edge " << adjacency_list[i][l] << " have the same color: " << edge_colors[i] << "\n";
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

     ofstream o("hca_solution.json");
     o << std::setw(4) << solutionjson << std::endl;

     cout << "Number of Colors in HCA Algorithm: " << colors_counter << "\n";

}