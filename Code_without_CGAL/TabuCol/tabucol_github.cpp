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
std::vector<std::vector<int>> adjacency_list, tabuTenure, adjColTab;
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
    for(int i=0;i<m;++i){
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
    for(int i=0;i<adjacency_list[nextMove.vertex].size();++i){
        --adjColTab[adjacency_list[nextMove.vertex][i]] [nextMove.oriCol];
        ++adjColTab[adjacency_list[nextMove.vertex][i]] [nextMove.nextCol];
    }
}

void tabuSearch() {
    iter=0;
    while(iter<maxIter){
        ++iter;
        findMove();
        makeMove();
        if (iter%50000==0) {cout << "f = " << f <<endl;}
        //cout << "f = " << f <<endl;
        if (iter%100000==0) {cout << "k = " << k << ", iter = " << iter <<endl;}
        if(f==0){
            cout<<"Success"<<endl;
            //output result
//            for(int i=0;i<g.vertexNum;++i){
//                cout<<i+1<<' '<<sol[i]<<endl;
//            }
            //verify
            for(int i=0;i<m;++i){
                for(int j=0;j<adjacency_list[i].size();++j){
                    if(sol[i]==sol[adjacency_list[i][j]]){
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
            for (int i=0; i<m; i++){
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

    std::ifstream file("instances/rsqrpecn8051.instance.json");
    Json::Reader reader;
    reader.parse( file, graphJson);

    n = graphJson["n"].asInt();
    m = graphJson["m"].asInt();
    k=300;

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

    //create adjacency list
    adjacency_list.clear();
    for (int i=0; i<m; i++){
        adjacency_list.push_back({});
        for (int j=0; j<m; j++){
            //if edge l and edge k intersect in their interiors, then push_back(1), otherwise push_back(0)
            //A=edge_i[l], B=edge_j[l], C=edge_i[k], D=edge_j[k]
            if (i==j) {}
            else if (doIntersect(edge_i[i], edge_j[i], edge_i[j], edge_j[j])){
                adjacency_list[i].push_back(j);
            } else {}
        }
    }

    //cout << "o";

    // std::ifstream file("dimacs_instances/flat1000_76_0.col.txt");
    // //dsjc1000.5: 89 (10 min), (no success for 87 after 2.5 hours)
    // //dsjc1000.1: 22 (1-2 seconds)
    // //dsjc1000.9: 
    // //dsjc500.5: 51 (1 min)
    // //dsjc250.5: 28 (1-2 min)
    // //flat300_28_0: 31 (4 minutes)
    // //le450_25c: 26 (a few seconds)
    // //flat1000_76_0: 89 (a few minutes)

    // //cout << "a";

    // n = 1000;
    // m = 1000;
    // int col_bound=m;
    // k=88;

    // //perform algorithm
    // //compute intersection graph as an adjacency matrix
    // //std::vector<std::vector<int>> adjacency_list;
    // adjacency_list.clear();
    // for (int l=0; l<m; l++){
    //     adjacency_list.push_back({});
    // }

    // //read file line by line

    // //cout << "q";

    // std::string str;
    // //std::stringstream ss;
    // while (getline(file,str)){
    // //while(std::cin >> str){
    //     //cout << str << "\n";
    //     std::stringstream ss;
    //     ss << str;
    //     int b, c;
    //     char a;
    //     ss >> a >> b >> c;
    //     //cin >> a >> b >> c;
    //     //cout << a << " " << b << " " << c << "\n";
    //     //cout << b << " " << c << "\n";
    //     adjacency_list[b-1].push_back(c-1);
    //     adjacency_list[c-1].push_back(b-1);
    //     //m++;
    // }
    // //cout << "x";

    //create tabucol algorithm, params : graph ,color num, maxIter

    while (k>0){

    f=0;
    //allocate memory
    sol.clear();
    for (int i=0; i<m; i++) {sol.push_back(0);}
//    bestSol=new int[g.vertexNum];
    tabuTenure.clear(); adjColTab.clear();
    for (int i=0; i<m; i++){
        tabuTenure.push_back({}); adjColTab.push_back({});
        for (int j=0; j<k; j++){
            tabuTenure[i].push_back(0); adjColTab[i].push_back(0);
        }
    }

    //select a solution randomly
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(0,k-1); // guaranteed unbiased
    for(int i=0;i<m;++i){
        int random_integer = uni(rng);
        sol[i]=random_integer;
    }

    //init Adjacent_Color_Table & f
    int color_i;
    int color_j;
    for(int i=0;i<m;++i){
        color_i=sol[i];
        for(int j=0; j<adjacency_list[i].size(); ++j){
            color_j=sol[adjacency_list[i][j]];
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