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
    for l in range(0,m):
        degrees[l]=len(adjacency_list[l])

    saturation = numpy.zeros(m)

    neighbors_colors = numpy.zeros((m,m))

    edge_colors = numpy.empty(m)
    edge_colors.fill(-1)
    current_edges = numpy.zeros(m)

    print("Initialization done")

    #pick sets of size 30 according to the dsatur rule and color these sets with ILP

    edges_colored=0
    current_bound=48

    while (edges_colored<m):
        #find the first 20 edges with highest saturation
        current_edges = numpy.zeros(m)
        ilp_bound = 2
        for i in range(0,ilp_bound):
            maxSat=-1
            maxDegree=-1
            current_vertex=0
            for j in range(0,m):
                if (current_edges[j]==0 and edge_colors[j]==-1):
                    if (saturation[j]>maxSat):
                        maxSat=saturation[j]
                        maxDegree=degrees[j]
                        current_vertex=j
                    elif (saturation[j]==maxSat and degrees[j]>maxDegree):
                        maxSat=saturation[j]
                        maxDegree=degrees[j]
                        current_vertex=j
            current_edges[current_vertex]=1

        #build ILP for current_edges
        coloring_found=0
        while (coloring_found==0):
            ilp_model = Model()
            variables = {}
            cross_con = {}
            delta = {}

            for i in range(0,m):
                if (current_edges[i]==1 and edge_colors[i]==-1):
                    variables[i]=ilp_model.addVar(lb=0, ub=current_bound-1, obj=0, vtype=GRB.INTEGER)
                elif (edge_colors[i]>=0):
                    variables[i]=ilp_model.addVar(lb=edge_colors[i], ub=edge_colors[i], obj=0, vtype=GRB.INTEGER)

            for i in range(0,m):
                if ((current_edges[i]==1 and edge_colors[i]==-1) or (edge_colors[i]>=0)):
                    for j in range(i+1,m):
                        if (((current_edges[j]==1 and edge_colors[j]==-1) or (edge_colors[j]>=0)) and (cross[i,j]==1)):
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
                    if (current_edges[k]==1 and edge_colors[k]==-1):
                        t=int(variables[k].x)
                        edge_colors[k]=t
                        edges_colored=edges_colored+1
                        print("Edges colored: " ,edges_colored)
                        #update saturation and neighbors_colors
                        for l in range(0, len(adjacency_list[k])):
                            if (neighbors_colors[adjacency_list[k][l]][t]==0):
                                neighbors_colors[adjacency_list[k][l]][t]=1
                                saturation[adjacency_list[k][l]]=saturation[adjacency_list[k][l]]+1


    































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

    with open('dsatur_with_sets_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)

    

if __name__ == "__main__":
    main()
