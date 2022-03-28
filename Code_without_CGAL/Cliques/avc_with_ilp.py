from gurobipy import *
import numpy
import json

def ccw(A,B,C):
    return (y[C]-y[A])*(x[B]-x[A]) > (y[B]-y[A])*(x[C]-x[A])

def intersect(A,B,C,D):
    return ccw(A,C,D) != ccw(B,C,D) and ccw(A,B,C) != ccw(A,B,D)

def main():
    #Read JSON Object and store values into arrays
    with open('instances/visp64932.instance.json') as f:
        graph_data = json.load(f)

    global x
    global y
    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]

    print("Loaded Data")

    #Build Constrints for ILP

    #For each edge i, compute cross[i]
    cross = numpy.zeros((m,m))#cross[i,j]==1 if edge i crosses edge j and i<j
    adjacency_list = {}
    for k in range(0,m):
        adjacency_list[k] = []
        for l in range(0,m):
            if (edge_i[k]==edge_i[l] or edge_i[k]==edge_j[l] or edge_j[k]==edge_i[l] or edge_j[k]==edge_j[l]):
                cross[k,l]=0
                cross[l,k]=0
            elif intersect(edge_i[k], edge_j[k], edge_i[l], edge_j[l]):
                cross[k,l]=1
                cross[l,k]=1
                adjacency_list[k].append(l)

    degrees = numpy.zeros(m)
    in_subgraph = numpy.ones(m)
    for l in range(0,m):
        for k in range(0,m):
            if (cross[l,k]==1):
                degrees[l]=degrees[l]+1


    graph_size=m
    clique_found=0
    while (graph_size>0 and clique_found==0):
        #find vertex v with smallest degree in subgraph
        min_degree=m
        v=0
        for i in range(0,m):
            if (in_subgraph[i]==1 and degrees[i]<min_degree):
                v=i
                min_degree=degrees[i]

        #if v has degree graph_size-1: clique_found=true
        if (min_degree==graph_size-1):
            clique_found=1
        else:
            in_subgraph[v]=0
            graph_size=graph_size-1
            for i in range(0, len(adjacency_list[v])):
                if (in_subgraph[adjacency_list[v][i]]==1):
                    degrees[adjacency_list[v][i]]=degrees[adjacency_list[v][i]]-1

    #check whether we can add one of the vertices which is not in the subgraph
    for i in range(0,m):
        if (in_subgraph[i]==0):
            in_clique=1
            c=graph_size
            for j in range(0, len(adjacency_list[i])):
                if (in_subgraph[adjacency_list[i][j]]):
                    c=c-1

            if (c>0):
                in_clique=0
            
            if (in_clique):
                in_subgraph[i]=1
                graph_size=graph_size+1

    #check that all vertices which are in_subgraph actually form a clique
    clique_check=1
    for i in range(0,m):
        if (in_subgraph[i]==1):
            #cout << "Degree of i: " << degrees[i] << "\n";
            v_counter=0
            for j in range(0, len(adjacency_list[i])):
                if (in_subgraph[adjacency_list[i][j]]==1):
                    v_counter=v_counter+1

            if (v_counter!=graph_size-1):
                clique_check=0
            

    edge_colors = numpy.empty(m)
    edge_colors.fill(-1)

    print (edge_colors)
    

    #the resulting independent set size is the size of one of the cliques in the intersection graph
    print("We found a valid clique in the graph: ", clique_check)
    print("Greedy Clique Size: ", graph_size)

    #color clique
    current_color=0
    in_clique = numpy.zeros(m)
    for i in range(0,m):
        if (in_subgraph[i]==1):
            edge_colors[i]=current_color
            current_color=current_color+1
            in_clique[i]=1



    #determine the number of neighbors in in_subgraph
    neighbors_in_subgraph = numpy.zeros(m)
    for i in range(0,m):
        temp=0
        for j in range(0, len(adjacency_list[i])):
            if (in_subgraph[adjacency_list[i][j]]==1):
                temp=temp+1

        neighbors_in_subgraph[i]=temp


    current_lower_bound=graph_size

    #now we add the vertices
    #from the remaining vertices, choose the with the most neighbors in in_subgraph
    #if we encounter a vertex which has at least current_lower_bound_neighbors in the subgraph,
    #then we check with the ILP whether we can color the graph with at most current_lower_bound colors
    #if it's not possible, then we increase current_lower_bound by 1
    changed_lower_bound=1
    for i in range(0,200):

        #pick vertex with most neighbors in subgraph
        max_neighbors_in_subgraph=0
        current_vertex=0
        for j in range(0,m):
            if (in_subgraph[j]==0 and neighbors_in_subgraph[j]>max_neighbors_in_subgraph):
                current_vertex=j
                max_neighbors_in_subgraph=neighbors_in_subgraph[j]


        if (max_neighbors_in_subgraph>=current_lower_bound):
            #check wether we can color the current subgraph with at most current_lower_bound colors

            #first check wether we can use the previous coloring and just add current_vertex
            # if (changed_lower_bound==0):
            #     possible_colors = numpy.zeros(m)
            #     c_counter=0
            #     for j in range(0, len(adjacency_list[current_vertex])):
            #         if (possible_colors[edge_colors[adjacency_list[current_vertex][j]]]==0):
            #             c_counter=c_counter+1
            #             possible_colors[edge_colors[adjacency_list[current_vertex][j]]]=1
            #     t=0
            #     while (possible_colors[t]==1):
            #         t=t+1
            
            #     if (t<current_lower_bound):
            #         edge_colors[current_vertex]=t
            # else:
            #use ilp to find new coloring
            ilp_model = Model()
            variables = {}
            cross_con = {}
            delta = {}

            variables[current_vertex]=ilp_model.addVar(lb=0, ub=current_lower_bound-1, obj=0, vtype=GRB.INTEGER)
            for j in range(0,m):
                if (in_subgraph[j]==1 and in_clique[j]==1):
                    variables[j]=ilp_model.addVar(lb=edge_colors[j], ub=edge_colors[j], obj=0, vtype=GRB.INTEGER)
                elif (in_subgraph[j]==1):
                    variables[j]=ilp_model.addVar(lb=0, ub=current_lower_bound-1, obj=0, vtype=GRB.INTEGER)

            for j in range(0,m):
                if (j==current_vertex or in_subgraph[j]):
                    for l in range(0,m):
                        if (l==current_vertex or in_subgraph[l]):
                            if (cross[j,l]==1 and (in_clique[j]==0 or in_clique[l]==0)):
                                delta[(j,l)]=ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
                                cross_con[(j,l)]=ilp_model.addConstr(variables[j]<=variables[l]-1+m*delta[(j,l)])
                                cross_con[(l,j)]=ilp_model.addConstr(variables[j]>=variables[l]+1-m*(1-delta[(j,l)]))

            ilp_model.update()
            ilp_model.optimize()

            if (ilp_model.status==GRB.Status.INFEASIBLE):
                current_lower_bound=current_lower_bound+1

        print("The lower bound is currently: ", current_lower_bound)
        print("We have this many vertices in the subgraph: ", graph_size)


        #update subgraph
        in_subgraph[current_vertex]=1
        graph_size=graph_size+1
        for j in range(0, len(adjacency_list[current_vertex])):
            neighbors_in_subgraph[adjacency_list[current_vertex][j]]=neighbors_in_subgraph[adjacency_list[current_vertex][j]]+1



    print("We determined the following lower bound for the graph: ", current_lower_bound)

if __name__ == "__main__":
    main()
