from gurobipy import *
import numpy
import json

edges_colored=0
edge_colors = numpy.empty(10)

def ccw(A,B,C):
    return (y[C]-y[A])*(x[B]-x[A]) > (y[B]-y[A])*(x[C]-x[A])

def intersect(A,B,C,D):
    return ccw(A,C,D) != ccw(B,C,D) and ccw(A,B,C) != ccw(A,B,D)

def dac(cons_edges):

    global x
    global y
    global n
    global m
    global edge_i
    global edge_j
    global edge_colors
    global considered_edges
    global edges_colored
    global cross
    global adjacency_list
    global saturation
    global neighbors_colors
    global degrees
    global actual_graph_adj_matrix

    #determine number of considered edges
    edge_counter=0
    for i in range(0,m):
        if (cons_edges[i]==1):
            edge_counter=edge_counter+1


    #if number of considered edges is less than 30, then solve with ILP
    if (edge_counter<=35):
        coloring_found=0
        current_bound=1
        while (coloring_found==0):
            ilp_model = Model()
            variables = {}
            cross_con = {}
            delta = {}

            for i in range(0,m):
                if (cons_edges[i]==1):
                    variables[i]=ilp_model.addVar(lb=0, ub=current_bound-1, obj=0, vtype=GRB.INTEGER)

            for i in range(0,m):
                if (cons_edges[i]==1):
                    for j in range(0,m):
                        if (cons_edges[j]==1 and cross[i,j]==1 and i<j):
                            delta[(j,i)]=ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
                            cross_con[(j,i)]=ilp_model.addConstr(variables[j]<=variables[i]-1+m*delta[(j,i)])
                            cross_con[(i,j)]=ilp_model.addConstr(variables[j]>=variables[i]+1-m*(1-delta[(j,i)]))

            ilp_model.update()
            ilp_model.optimize()

            if (ilp_model.status==GRB.Status.INFEASIBLE):
                current_bound=current_bound+1
            else:
                coloring_found=1
                for k in range(0,m):
                    if (cons_edges[k]==1):
                        t=int(variables[k].x)
                        edge_colors[k]=t
                        edges_colored=edges_colored+1
                        #print("Edge colored: " ,k)
                        #print("Last color: ", t)
                        #update saturation and neighbors_colors
                        for l in range(0, len(adjacency_list[k])):
                            if (neighbors_colors[adjacency_list[k][l]][t]==0):
                                neighbors_colors[adjacency_list[k][l]][t]=1
                                saturation[adjacency_list[k][l]]=saturation[adjacency_list[k][l]]+1


    #else define separating line and divie edges into left, right and middle
    else:

        left_cons_edges = numpy.zeros(m)
        right_cons_edges = numpy.zeros(m)
        new_cons_edges = cons_edges


        # compute min_cut of the subgraph induced by the edges in cons_edges
        print("Starting min-cut algorithm")

        #determine vertices which are part of the subgraph induced by the edges in cons_edges
        cons_vertices = numpy.zeros(n)
        for i in range(0,m):
            if (cons_edges[i]==1):
                cons_vertices[edge_i[i]]=1
                cons_vertices[edge_j[i]]=1

        n_vertices=0
        vertex_map = []

        for i in range(0,n):
            if (cons_vertices[i]==1):
                n_vertices=n_vertices+1
                vertex_map.append(i)

        #pair<int, vector<int>> best = {INT_MAX, {}}
        best_cut_size = m*m
        best_cut = []
        co = {}
        mat = {}
        for i in range(0,n_vertices):
            mat[i]=[]
            for j in range(0,n_vertices):
                mat[i].append(int(actual_graph_adj_matrix[vertex_map[i], vertex_map[j]]))
        for i in range(0,n_vertices):
            co[i] = [i]
        for ph in range(1,n_vertices):
            #print("ph=", ph)
            w = mat[0]
            s = 0
            t = 0
            for it in range(0,n_vertices-ph):
                #print("ph=", ph)
                #print("it=", it)
                w[int(t)] = int(-m*m)
                s = t 
                t = 0 #t = max_element(w.begin(), w.end()) - w.begin()
                max_w=int(-m*m)
                for i in range(0,n_vertices):
                    if (w[i]>max_w):
                        t=i
                        max_w=w[i]
                for i in range(0,n_vertices):
                    #print("i=", i)
                    w[i] = w[i] + mat[int(t)][i]
                    #print("t=", t)
                    #print("i=", i)
                    #print(mat[int(t)][i])
            #best = min(best, {w[t] - mat[t][t], co[t]})
            if (best_cut_size>w[int(t)]-mat[int(t)][int(t)]):
                best_cut_size=w[int(t)]-mat[int(t)][int(t)]
                best_cut=co[int(t)]
                best_cut = [x for x in best_cut if best_cut.count(x) == 1]
            #co[s].insert(co[s].end(), co[t].begin(), co[t].end())
            for i in range(0,len(co[int(t)])):
                co[int(s)].append(co[int(t)][i])
            co[int(s)] = [x for x in co[int(s)] if co[int(s)].count(x) == 1]
            for i in range(0,n_vertices):
                mat[int(s)][i] = mat[int(s)][i]+mat[int(t)][i]
            for i in range(0,n_vertices):
                mat[i][int(s)] = mat[int(s)][i]
            mat[0][int(t)] = 0#int(-m*m)

        print("Finished min-cut-algorithm")
        print("Best cut:")
        for i in range(0,len(best_cut)):
            print(best_cut[i], vertex_map[best_cut[i]])
        print("Best cut Size: ", best_cut_size)


        #now that we know which vertices are on one side and on the other side of the cut, we can partition the edges
        in_left_cut = numpy.zeros(n)
        for i in range(0,len(best_cut)):
            in_left_cut[vertex_map[best_cut[i]]]=1

        left_cons_edges_counter=0
        right_cons_edges_counter=0

        #e = edge_counter


        for i in range(0,m):
            if (cons_edges[i]==1):
                if (in_left_cut[edge_i[i]]==1 and in_left_cut[edge_j[i]]==1):
                    left_cons_edges[i]=1
                    new_cons_edges[i]=0
                    right_cons_edges[i]=0
                    edge_counter=edge_counter-1
                    left_cons_edges_counter=left_cons_edges_counter+1
                elif (in_left_cut[edge_i[i]]==0 and in_left_cut[edge_j[i]]==1):
                    new_cons_edges[i]=1
                    right_cons_edges[i]=0
                    left_cons_edges[i]=0
                elif (in_left_cut[edge_i[i]]==1 and in_left_cut[edge_j[i]]==0):
                    new_cons_edges[i]=1
                    right_cons_edges[i]=0
                    left_cons_edges[i]=0
                else:
                    right_cons_edges[i]=1
                    new_cons_edges[i]=0
                    left_cons_edges[i]=0
                    edge_counter=edge_counter-1
                    right_cons_edges_counter=right_cons_edges_counter+1





        # if (right_cons_edges_counter+left_cons_edges_counter+edge_counter==e):
        #     print("Divide is valid")
        # else:
        #     print("Divide is invalid")

        print("Making recursive calls")
        dac(left_cons_edges)
        dac(right_cons_edges)

        #color edges which intersect the sepearting line with DSatur

        while(edge_counter>0):
            #find vertex with highest_saturation
            current_vertex=0
            maxSat=-1
            maxDegree=-1
            for k in range(0,m):
                if (new_cons_edges[k]==1):
                    if (saturation[k]>maxSat):
                        current_vertex=k
                        maxSat=saturation[k]
                        maxDegree=degrees[k]
                    elif (saturation[k]==maxSat and degrees[k]>maxDegree):
                        current_vertex=k
                        maxSat=saturation[k]
                        maxDegree=degrees[k]

            #color current_vertex with lowest possible_color
            possible_colors = numpy.zeros(m)
            for k in range(0, len(adjacency_list[current_vertex])):
                if (edge_colors[adjacency_list[current_vertex][k]]>=0):
                    possible_colors[int(edge_colors[adjacency_list[current_vertex][k]])]=1

            t=0
            while (possible_colors[t]==1):
                t=t+1
            edge_colors[current_vertex]=t

            #update saturation and neighbors colors
            for l in range(0, len(adjacency_list[current_vertex])):
                if (neighbors_colors[adjacency_list[current_vertex][l]][int(t)]==0):
                    neighbors_colors[adjacency_list[current_vertex][l]][int(t)]=1
                    saturation[adjacency_list[current_vertex][l]]=saturation[adjacency_list[current_vertex][l]]+1

            new_cons_edges[current_vertex]=0
            edges_colored=edges_colored+1
            #print("Edge colored: " , current_vertex)
            #print("Last color: ", t)
            edge_counter=edge_counter-1

    # color_class_size = numpy.zeros(m)
    # for i in range(0,m):
    #     if (cons_edges[i]==1):
    #         color_class_size[int(edge_colors[i])]=color_class_size[int(edge_colors[i])]+1

    # colors_with_size = []
    # for i in range(0,m):
    #     #if (cons_edges[i]==1):
    #     colors_with_size.append((i,color_class_size[i]))

    # colors_with_size.sort(key = lambda x: -x[1])

    # color_mapping = numpy.zeros(m)
    # for i in range(0,m):
    #     #if (cons_edges[i]==1):
    #     color_mapping[colors_with_size[i][0]]=i

    # for i in range(0,m):
    #     if (cons_edges[i]==1):
    #         edge_colors[i]=color_mapping[int(edge_colors[i])]





def main():
    #Read JSON Object and store values into arrays
    with open('instances/rvisp3499.instance.json') as f:
        graph_data = json.load(f)

    global x
    global y
    global n
    global m
    global edge_i
    global edge_j
    global edge_colors
    global considered_edges
    global edges_colored
    global cross
    global adjacency_list
    global saturation
    global neighbors_colors
    global degrees
    global actual_graph_adj_matrix

    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]

    print("Loaded Data")

    #build adjacency matrix for actual graph
    actual_graph_adj_matrix = numpy.zeros((n,n))
    for i in range(0,m):
        actual_graph_adj_matrix[edge_i[i], edge_j[i]]=int(1)
        actual_graph_adj_matrix[edge_j[i], edge_i[i]]=int(1)

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
    for l in range(0,m):
        for k in range(0,m):
            if (cross[l,k]==1):
                degrees[l]=degrees[l]+1

    saturation = numpy.zeros(m)

    neighbors_colors = numpy.zeros((m,m))

    edge_colors = numpy.empty(m)
    edge_colors.fill(-1)
    considered_edges = numpy.ones(m)

    print("Initialization done")

    edges_colored=0

    dac(considered_edges)

    color_counter=0
    for i in range(0,m):
        if (edge_colors[i]+1>color_counter):
            color_counter=edge_colors[i]+1








    #check whether color assignment is valid
    valid_color_assignment=1
    for k in range (0,m):
        if (edge_colors[k]==-1):
            valid_color_assignment=0
        for l in range (0, len(adjacency_list[k])):
                if (edge_colors[k]==edge_colors[adjacency_list[k][l]]):
                    valid_color_assignment=0
                    print("The following edges have the same color", k, adjacency_list[k][l], edge_colors[k])


    print("Color Assignment is valid: ", valid_color_assignment)
    print("This many colors were used: ", color_counter)



    edge_colors_list = []
    for k in range(0,m):
        edge_colors_list.append(int(edge_colors[k]))

    print("Starting to write JSON file")


    #Write JSON solution file
    ilp_json = {"type" : "Solution_CGSHOP2022", "instance" : "", "num_colors" : 0, "colors": []}
    ilp_json["num_colors"]=int(color_counter)
    print("Added color_counter")
    ilp_json["instance"]=graph_data["id"]
    print("Added id")
    ilp_json["colors"]=edge_colors_list
    print("Added edge_colors_list")

    with open('divide_and_conquer2_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)

    

if __name__ == "__main__":
    main()
