//
// Created by Guan on 2020/9/14.
//

#include <iostream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <fstream>
#include <iomanip>
#include <climits>
#include <cstring>
#include <vector>
#include <random>

using std::cout;
using std::endl;

int n,m,k, maxIter, bestF, iter, f;
std::vector<long> x,y;
std::vector<std::pair<int, int>> edges;
std::vector<std::vector<int>> s_adjacency_list, tabuTenure, adjColTab;
std::vector<int> sol, edge_i, edge_j;
Json::Value graphJson;
struct Move{
    int vertex;
    //original color
    int oriCol;
    //next color
    int nextCol;
    int deltaF;
}nextMove;

bool ccw(int A, int B, int C){
    //return (C.y-A.y) * (B.x-A.x) > (B.y-A.y) * (C.x-A.x)
    return ((y[C]-y[A])*(x[B]-x[A])) > ((y[B]-y[A])*(x[C]-x[A]));
}

// Return true if line segments AB and CD intersect
bool doIntersect(int A, int B, int C, int D){
    if ((B==C) || (B==D) || (A==C) || (A==D)) {return false;}
    else {return ((ccw(A,C,D)!=ccw(B,C,D)) && (ccw(A,B,C)!=ccw(A,B,D)));}
}

void findMove() {
    //There may be some moves with the same deltaF.
    struct move{
        int vertex;
        int color;
    };
    std::vector<struct move> tabuMoveList,nontabuMoveList;
    int tabuMinDeltaF=INT_MAX;
    int nontabuMinDeltaF=INT_MAX;
    int tmpDeltaF;
    for(int i=0;i<n;++i){
        if(adjColTab[i][sol[i]] > 0){
            for(int l=0;l<k;++l){
                if(l!=sol[i]){
                    tmpDeltaF=adjColTab[i][l]-adjColTab[i][sol[i]];
                    if(tabuTenure[i][l] < iter){ //< or <= ?
                        //update the non-tabu best move
                        if(tmpDeltaF <= nontabuMinDeltaF){
                            if(tmpDeltaF < nontabuMinDeltaF){
                                nontabuMoveList.clear();
                                nontabuMinDeltaF=tmpDeltaF;
                            }
                            struct move tmp={i,l};
                            nontabuMoveList.push_back(tmp);
                        }
                    }else{
                        //update the tabu best move
                        if(tmpDeltaF <= tabuMinDeltaF){
                            if(tmpDeltaF < tabuMinDeltaF){
                                tabuMoveList.clear();
                                tabuMinDeltaF=tmpDeltaF;
                            }
                            struct move tmp={i,l};
                            tabuMoveList.push_back(tmp);
                        }
                    }
                }
            }
        }
    }
//    Aspiration criterion: it holds if both the following two conditions satisfy:
//    1. the best tabu move is better than the previous best known solution;
//    2. the best tabu move is better than the best non-tabu move in the current neighborhood.
    if((f+tabuMinDeltaF)<bestF && tabuMinDeltaF < nontabuMinDeltaF){
        nextMove.deltaF=tabuMinDeltaF;
        //select a move randomly
        int randIndex= rand() % tabuMoveList.size();
        nextMove.vertex=tabuMoveList[randIndex].vertex;
        nextMove.oriCol=sol[tabuMoveList[randIndex].vertex];
        nextMove.nextCol=tabuMoveList[randIndex].color;
    }else{
        nextMove.deltaF=nontabuMinDeltaF;
        int randIndex= rand() % nontabuMoveList.size();
        nextMove.vertex = nontabuMoveList[randIndex].vertex;
        nextMove.oriCol=sol[nextMove.vertex];
        nextMove.nextCol=nontabuMoveList[randIndex].color;
    }
}

void makeMove() {
    //update solution ,f and tabuTenure
    sol[nextMove.vertex]=nextMove.nextCol;
    f = f + nextMove.deltaF;
    if(f < bestF){
        bestF = f;
    }
    tabuTenure[nextMove.vertex][nextMove.oriCol]=iter+f+rand()%10;
    //Update the Adjacent_Color_Table;
    for(int i=0;i<s_adjacency_list[nextMove.vertex].size();++i){
        --adjColTab[s_adjacency_list[nextMove.vertex][i]] [nextMove.oriCol];
        ++adjColTab[s_adjacency_list[nextMove.vertex][i]] [nextMove.nextCol];
    }
}

void tabuSearch() {
    iter=0;
    while(iter<maxIter){
        ++iter;
        findMove();
        makeMove();
        if (iter%100==0) {cout << "f = " << f <<endl;}
        //cout << "f = " << f <<endl;
        if (iter%100==0) {cout << "k = " << k << ", iter = " << iter <<endl;}
        if(f==0){
            cout<<"Success"<<endl;
            //output result
//            for(int i=0;i<g.vertexNum;++i){
//                cout<<i+1<<' '<<sol[i]<<endl;
//            }
            //verify
            for(int i=0;i<n;++i){
                for(int j=0;j<s_adjacency_list[i].size();++j){
                    if(sol[i]==sol[s_adjacency_list[i][j]]){
                        cout<<"Error"<<endl;
                    }
                }
            }
            //output Number of Iterations
            cout<<"Number of Iterations: "<<iter<<endl;
            //write solutionjson
            Json::Value solutionjson;
            solutionjson["type"] = "Solution_CGSHOP2022";
            solutionjson["instance"] = graphJson["id"];
            solutionjson["num_colors"] = k;
    
            Json::Value vec(Json::arrayValue);
            for (int i=0; i<n; i++){
                vec.append(Json::Value(sol[i]));
            }
            solutionjson["colors"] = vec;

            std::ofstream o("tabucol_solution.json");
            o << std::setw(4) << solutionjson << std::endl;
            cout << "Color Assignment of TabuCol is valid: Yes" << endl;
            cout << "Number of Colors in TabuCol Algorithm: " << k << "\n";
            return;
        }
    }
    cout<<"Not Found. f = " << f <<endl;
    cout<<"Number of Iterations: "<<iter<<endl;
}

int main() {
    //Read JSON Object and store values into vectors
    //Using fstra, to get the file pointer in "file"

    std::ifstream file("instances/visp64932.instance.json");
    Json::Reader reader;
    reader.parse( file, graphJson);

    n = graphJson["n"].asInt();
    m = graphJson["m"].asInt();
    k=492;

    x.clear(); y.clear();
    for (int i=0; i<n; i++){
        x.push_back(graphJson["x"][i].asInt64());
        y.push_back(graphJson["y"][i].asInt64());
    }
    
    //edge id's start at 0
    edges.clear();edge_i.clear();edge_j.clear();

    for (int i=0; i<m; i++){
        edge_i.push_back(graphJson["edge_i"][i].asInt());
        edge_j.push_back(graphJson["edge_j"][i].asInt());
        edges.push_back(std::make_pair(edge_i[i], edge_j[i]));
    }

    //compute complement intersection graph as an adjacency matrix
    std::vector<std::vector<int>> adjacency_list, adj_matrix; //adjacency_list_2;
    adjacency_list.clear(); adj_matrix.clear(); //adjacency_list_2.clear();
    for (int l=0; l<m; l++){
        adjacency_list.push_back({});
        adj_matrix.push_back({});
        for (int k=0; k<m; k++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (l==k) {adj_matrix[l].push_back(0);}
            else if (doIntersect(edge_i[l], edge_j[l], edge_i[k], edge_j[k])){
                adjacency_list[l].push_back(k);
                adj_matrix[l].push_back(1);
            } else {adj_matrix[l].push_back(0);}
        }
    }

    //build star intersection graph (adjacency matrix)
    std::vector<std::vector<int>> star_intersection_graph; star_intersection_graph.clear();
    for (int i=0; i<n; i++){
        star_intersection_graph.push_back({});
        for (int j=0; j<n; j++){
            star_intersection_graph[i].push_back(0);
        }
    }

    for (int i=0; i<m; i++){
        for (int j=0; j<adjacency_list[i].size(); j++){
            int k=adjacency_list[i][j];
            int i_i=edge_i[i];
            int i_j=edge_j[i];
            int k_i=edge_i[k];
            int k_j=edge_j[k];

            star_intersection_graph[i_i][k_i]=1;
            star_intersection_graph[i_i][k_j]=1;
            star_intersection_graph[i_j][k_i]=1;
            star_intersection_graph[i_j][k_j]=1;

            star_intersection_graph[k_i][i_i]=1;
            star_intersection_graph[k_j][i_i]=1;
            star_intersection_graph[k_i][i_j]=1;
            star_intersection_graph[k_j][i_j]=1;
        }
    }

    //build adjacency_list
    s_adjacency_list.clear();
    for (int i=0; i<n; i++){
        s_adjacency_list.push_back({});
        for (int j=0; j<n; j++){
            if (star_intersection_graph[i][j]==1){
                s_adjacency_list[i].push_back(j);
            }
        }
    }
























    //create tabucol algorithm, params : graph ,color num, maxIter

    std::cout << "x" << "\n";
    //m=n;

    while (k>0){

    f=0;
    //allocate memory
    sol.clear();
    for (int i=0; i<n; i++) {sol.push_back(0);}
//    bestSol=new int[g.vertexNum];
    tabuTenure.clear(); adjColTab.clear();
    for (int i=0; i<n; i++){
        tabuTenure.push_back({}); adjColTab.push_back({});
        for (int j=0; j<k; j++){
            tabuTenure[i].push_back(0); adjColTab[i].push_back(0);
        }
    }

    //select a solution randomly
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(0,k-1); // guaranteed unbiased
    for(int i=0;i<n;++i){
        int random_integer = uni(rng);
        sol[i]=random_integer;
    }

    //init Adjacent_Color_Table & f
    int color_i;
    int color_j;
    for(int i=0;i<n;++i){
        color_i=sol[i];
        for(int j=0; j<s_adjacency_list[i].size(); ++j){
            color_j=sol[s_adjacency_list[i][j]];
            if(color_i==color_j){
                ++f;
            }
            ++adjColTab[i][color_j];
        }
    }
    f/=2;
    bestF=f;

    //k=75;
    maxIter=100000000;
    tabuSearch();
    k--;
    }
    return 0;
}