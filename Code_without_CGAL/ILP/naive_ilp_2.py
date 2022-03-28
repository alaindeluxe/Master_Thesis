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
    greedy_colors=102
    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]

    print("Loaded Data")

    ilp_model = Model()
    #xi==j if edge i has color j
    variables = {}
    for k in range(0,m):
        variables[k]=ilp_model.addVar(lb=0, ub=greedy_colors-1, obj=0, vtype=GRB.INTEGER)
        print("added variable", k)

    ilp_model.update()

    #Build Constrints for ILP

    #For each edge i, compute cross[i]
    cross = numpy.zeros((m,m))#cross[i,j]==1 if edge i crosses edge j and i<j
    for k in range(0,m):
        for l in range(k+1,m):
            if (edge_i[k]==edge_i[l] or edge_i[k]==edge_j[l] or edge_j[k]==edge_i[l] or edge_j[k]==edge_j[l]):
                cross[k,l]=0
            elif intersect(edge_i[k], edge_j[k], edge_i[l], edge_j[l]):
                cross[k,l]=1
            print("computed intersection for ",k,l)
    #print(cross)

    #if two edges cross, they need to have different colors
    cross_con = {}
    delta = {}
    for k in range(0,m):
        for l in range(k+1,m):
            if (cross[k,l]==1):
                #add constraint that xk!=xl
                delta[(k,l)]=ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.INTEGER)
                cross_con[(k,l)]=ilp_model.addConstr(variables[k]<=variables[l]-1+n*delta[(k,l)])
                cross_con[(l,k)]=ilp_model.addConstr(variables[k]>=variables[l]+1-n*(1-delta[(k,l)]))
                print("added unequal constraint for ",k,l)
    ilp_model.update()

    #Run ILP
    #ilp_model.Params.TimeLimit = 300 #seconds
    print("starting optimization of model")
    ilp_model.optimize()#minimizes the objective function
    print("finished optimization of model")

    #Translate ILP result into the JSON data
    #Compute edge_colors
    edge_colors = numpy.zeros(m) #edge_color[i] is the color of edge i
    used_colors = numpy.zeros(greedy_colors) #if color l has already been used for at least one edge, then used_colors[l]=1
    for k in range(0,m):
        t=int(variables[k].x)
        edge_colors[k]=t
        #print("t=",t)
        used_colors[t]=1

    print("calculated edge_colors and used_colors")

    #count number of colors
    color_counter=0
    for k in range(0,greedy_colors):
        if (used_colors[k]==1):
            color_counter=color_counter+1

    print("Computed color_counter")

    edge_colors_list = []
    for k in range(0,m):
        edge_colors_list.append(int(edge_colors[k]))

    print("Starting to write JSON file")


    #Write JSON solution file
    ilp_json = {"type" : "Solution_CGSHOP2022", "instance" : "", "num_colors" : 0, "colors": []}
    ilp_json["num_colors"]=color_counter
    print("Added color_counter")
    ilp_json["instance"]=graph_data["id"]
    print("Added id")
    ilp_json["colors"]=edge_colors_list
    print("Added edge_colors_list")

    with open('naive_ilp_2_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)



if __name__ == "__main__":
    main()
