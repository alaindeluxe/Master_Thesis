from gurobipy import *
import numpy
import json

def ccw(A,B,C):
    return (y[C]-y[A])*(x[B]-x[A]) > (y[B]-y[A])*(x[C]-x[A])

def intersect(A,B,C,D):
    return ccw(A,C,D) != ccw(B,C,D) and ccw(A,B,C) != ccw(A,B,D)

def main():

    #Read JSON Object and store values into arrays
    with open('instances/vispecn2518.instance.json') as f:
        graph_data = json.load(f)

    global x
    global y

    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]
    colors_upper_bound=70

    print("Loaded Data")

    # ilp_model = Model()
    # #xi==j if edge i has color j
    # variables = {}
    # for k in range(0,m):
    #     variables[k]=ilp_model.addVar(lb=0, ub=greedy_colors-1, obj=0, vtype=GRB.INTEGER)
    #     print("added variable", k)

    # ilp_model.update()

    #Build Constrints for ILP

    #For each edge i, compute cross[i]
    adjacency_list={}
    cross = numpy.zeros((m,m))#cross[i,j]==1 if edge i crosses edge j and i<j
    for k in range(0,m):
        current_list = []
        for l in range(0,m):
            if (edge_i[k]==edge_i[l] or edge_i[k]==edge_j[l] or edge_j[k]==edge_i[l] or edge_j[k]==edge_j[l]):
                cross[k,l]=0
                cross[l,k]=0
            elif intersect(edge_i[k], edge_j[k], edge_i[l], edge_j[l]):
                cross[k,l]=1
                cross[l,k]=1
                current_list.append(l)
            #print("computed intersection for ",k,l)
        adjacency_list[k]=current_list

    print("Computed adjacency list")

    #compute degrees
    degrees=numpy.zeros(m)
    for k in range(0,m):
        for l in range(0,m):
            if (cross[k,l]==1):
                degrees[k]=degrees[k]+1

    #run DSatur
    #initaialize saturation
    saturation=numpy.zeros(m)#std::vector<int> saturation; saturation.clear();//saturation of a vertex
    neighbors_colors=numpy.zeros((m,k))#std::vector<std::vector<int>> neighbors_colors; neighbors_colors.clear();//neighbors_colors[i][j]==1 if vertex i has a neighbors with color j

    maxSat=0
    maxDegree=0
    maxSat_vertex=0
    for k in range(1,m):
        if degrees[k]>degrees[maxSat_vertex]:
            maxSat_vertex=k
    i=0

    colors_counter=0
    edge_colors=numpy.zeros(m)#std::vector<int> edge_colors; edge_colors.clear();
    used_colors=[]#std::vector<int> used_colors; used_colors.clear();//contains all colors that have already been assigned
    for k in range (0,m):
        edge_colors[k]=-1

    while(i<m):
        current_vertex=maxSat_vertex
        possible_colors=numpy.zeros(colors_upper_bound)#std::vector<int> possible_colors; possible_colors.clear();

        for j in range(0,len(adjacency_list[current_vertex])):
            if (edge_colors[adjacency_list[current_vertex][j]]>=0):
                #print(edge_colors[adjacency_list[current_vertex][j]])
                possible_colors[int(edge_colors[adjacency_list[current_vertex][j]])]=1
        t=0
        while (t<len(used_colors) and possible_colors[used_colors[t]]==1):
            t=t+1
        if (t<len(used_colors)):
            edge_colors[current_vertex]=used_colors[t]
            for j in range (0,len(adjacency_list[current_vertex])):
                if (neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]==0):
                    neighbors_colors[adjacency_list[current_vertex][j]][used_colors[t]]=1
                    saturation[adjacency_list[current_vertex][j]]=saturation[adjacency_list[current_vertex][j]]+1
        else:
            t=0
            while (possible_colors[t]>0):
                t=t+1
            edge_colors[current_vertex]=t
            used_colors.append(t)
            colors_counter=colors_counter+1
            for j in range(0,len(adjacency_list[current_vertex])):
                if (neighbors_colors[adjacency_list[current_vertex][j]][t]==0):
                    neighbors_colors[adjacency_list[current_vertex][j]][t]=1
                    saturation[adjacency_list[current_vertex][j]]=saturation[adjacency_list[current_vertex][j]]+1

        #find vertex with highest saturation
        maxSat=0; maxDegree=0
        for j in range(0,m):
            if (edge_colors[j]==-1):
                if (saturation[j]>maxSat):
                    maxSat_vertex=j
                    maxSat=saturation[j]
                    maxDegree=degrees[j]
                elif (saturation[j]==maxSat and degrees[j]>maxDegree):
                    maxSat_vertex=j
                    maxSat=saturation[j]
                    maxDegree=degrees[j]

        i=i+1



    #sort color classes such that the biggest color classes have small color
    color_class_size = numpy.zeros(colors_counter)
    for i in range(0,m):
        color_class_size[int(edge_colors[i])]=color_class_size[int(edge_colors[i])]+1

    colors_with_size = []
    for i in range(0,colors_counter):
        colors_with_size.append((i,color_class_size[i]))

    colors_with_size.sort(key = lambda x: -x[1])
    #print(colors_with_size)

    color_mapping = numpy.zeros(colors_counter)
    for i in range(0, colors_counter):
        color_mapping[colors_with_size[i][0]]=i

    print(color_mapping)

    for i in range(0,m):
        edge_colors[i]=color_mapping[int(edge_colors[i])]


    ilp_model = Model()
    #xi==j if edge i has color j
    #for each vertex which has a color below a certain threshold, add a variable for it
    colors_threshold=50
    #50 nodes and 245 edges takes 3 minutes with naive ILP
    #61 nodes and 368 edges takes more than 8 hours with naive ILP
    colors_counter=66
    #ilp_model.Params.SolutionLimit = 1
    ilp_model.update()
    variables = {}
    high_color_vertices = []
    for k in range(0,m):
        if edge_colors[k]>=colors_threshold:
            high_color_vertices.append(k)
            variables[k]=ilp_model.addVar(lb=colors_threshold, ub=colors_counter-1, obj=0, vtype=GRB.INTEGER)

    ilp_model.update()

    #add constraints
    cross_con = {}
    delta = {}
    t = {}
    number_of_constraints=0
    for k in high_color_vertices:
        for l in high_color_vertices:
            if (cross[k,l]==1 and k<l):
                #add constraint that xk!=xl
                # delta[(k,l)]=ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.INTEGER)
                # cross_con[(k,l)]=ilp_model.addConstr(variables[k]<=variables[l]-1+m*delta[(k,l)])
                # cross_con[(l,k)]=ilp_model.addConstr(variables[k]>=variables[l]+1-m*(1-delta[(k,l)]))
                #print("added unequal constraint for ",k,l)

                #ILP version 2
                delta[(k,l)]=ilp_model.addVar(obj=-1, vtype=GRB.INTEGER)
                t[(k,l)]=ilp_model.addVar(obj=0, vtype=GRB.INTEGER)
                ilp_model.addConstr(t[(k,l)]>=variables[k]-variables[l])
                ilp_model.addConstr(t[(k,l)]<=variables[k]-variables[l])
                cross_con[(k,l)]=ilp_model.addGenConstrAbs(delta[(k,l)], t[(k,l)])
                number_of_constraints=number_of_constraints+1
    ilp_model.update()

    print("Number of constraints: ", number_of_constraints)

    print("starting optimization of model")
    ilp_model.optimize()#minimizes the objective function
    print("finished optimization of model")

    colors_counter=colors_threshold #to count the new number of colors
    for k in high_color_vertices:
        t=int(variables[k].x)
        edge_colors[k]=t
        if (t>colors_counter-1):
            colors_counter=t+1
        #print("t=",t)


    #check whether we have a correct color assignment
    errors=0
    valid_color_assignment=1
    for k in range(0,m):
        for l in range(0,len(adjacency_list[k])):
            if (edge_colors[k]==edge_colors[adjacency_list[k][l]]):
                    valid_color_assignment=0
                    errors=errors+1
                    #print("Two crossing edges have the same color")

    print("Errors: ", errors/2)

    edge_colors_list = []
    for k in range(0,m):
        edge_colors_list.append(int(edge_colors[k]))

    print("Starting to write JSON file")


    #Write JSON solution file
    ilp_json = {"type" : "Solution_CGSHOP2022", "instance" : "", "num_colors" : 0, "colors": []}
    ilp_json["num_colors"]=colors_counter
    print("Added color_counter: color_counter=", colors_counter)
    ilp_json["instance"]=graph_data["id"]
    print("Added id")
    ilp_json["colors"]=edge_colors_list
    print("Added edge_colors_list")

    with open('dsatur_with_ilp_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)



if __name__ == "__main__":
    main()
