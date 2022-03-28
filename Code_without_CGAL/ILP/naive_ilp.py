from gurobipy import *
import numpy
import json

def ccw(A,B,C):
    return (y[C]-y[A])*(x[B]-x[A]) > (y[B]-y[A])*(x[C]-x[A])

def intersect(A,B,C,D):
    return ccw(A,C,D) != ccw(B,C,D) and ccw(A,B,C) != ccw(A,B,D)

def main():

    #Read JSON Object and store values into arrays
    with open('instances/rvisp3499.instance.json') as f:
        graph_data = json.load(f)

    global x
    global y
    greedy_colors=100
    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]

    print("Loaded Data")
    #print(x[2]) works!!

    #Build Variables for Example ILP
    #GRB:INFINITY is highest possible number
    #n=8
    #m=4
    #x=[0,2,3,4,5,5,6,6]
    #y=[0,3,5,2,4,-1,6,3]
    #edge_i=[0,1,2,4]
    #edge_j=[6,7,3,5]

    ilp_model = Model()
    #xij==1 if edge i has color j
    #x00 = ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
    variables = {}
    for k in range(0,m):
        for l in range(0,greedy_colors):
            variables[(k,l)]=ilp_model.addVar(lb=0, ub=1, obj=l, vtype=GRB.BINARY)
            print("added variable", k,l)

    ilp_model.update()

    #Build Constrints for ILP
    #Each edge has to have at least one color
    L = []
    for k in range(0,greedy_colors):
        L.append(k)
    con = {}
    for k in range(0,m):
        con[k]=ilp_model.addConstr(quicksum(variables[k,l] for l in L)>=1)
        print("added >=1 constraint for edge ",k)
    #con0 = ilp_model.addConstr(x00+x01+x02+x03+x04+x05+x06+x07>=1)
    ilp_model.update()

    #For each edge i, compute cross[i]
    cross = numpy.zeros((m,m))#cross[i,j]==1 if edge i crosses edge j
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
    for k in range(0,m):
        L = []
        for l in range(k+1,m):
            if (cross[k,l]==1):
                L.append(l)
        for t in range(0,greedy_colors):
            cross_con[(k,t)]=ilp_model.addConstr(variables[k,t]+quicksum(variables[l,t] for l in L)<=1)
            print("added <=1 constraint", k,t)
    ilp_model.update()

    #cross_con01_0 = ilp_model.addConstr(x00+x10<=1) #edges 0 and 1 cross, therefore they can't both have color 0
    #cross_con01_1 = ilp_model.addConstr(x01+x11<=1)
    #cross_con01_2 = ilp_model.addConstr(x02+x12<=1)
    #cross_con01_3 = ilp_model.addConstr(x03+x13<=1)

    #Run ILP
    print("starting optimization of model")
    ilp_model.optimize()#minimizes the objective function
    #x00.x, x01.x contain the optimal value of the variables
    print("finished optimization of model")
    


    #Translate ILP result into the JSON data
    #Compute edge_colors
    edge_colors = numpy.zeros(m) #edge_color[i] is the color of edge i
    used_colors = numpy.zeros(greedy_colors) #if color l has already been used for at least one edge, then used_colors[l]=1
    for k in range(0,m):
        edge_colors[k]=-1

    print("initialized edge_colors")
    

    #go over variables and extract the color assignments
    for k in range(0,m):
        l=0
        while(l<greedy_colors and int(variables[(k,l)].x)==0):
            l=l+1
        edge_colors[k]=l
        used_colors[l]=1
        print("Computed color for edge ", k)

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

    with open('naive_ilp_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)



if __name__ == "__main__":
    main()
