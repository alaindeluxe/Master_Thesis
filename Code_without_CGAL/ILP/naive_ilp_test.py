from gurobipy import *
import numpy
import json

def ccw(A,B,C):
    return (y[C]-y[A])*(x[B]-x[A]) > (y[B]-y[A])*(x[C]-x[A])

def intersect(A,B,C,D):
    return ccw(A,C,D) != ccw(B,C,D) and ccw(A,B,C) != ccw(A,B,D)

def main():

    #Read JSON Object and store values into arrays
    with open('instances/reecn3382.instance.json') as f:
        graph_data = json.load(f)

    global x
    global y
    n=graph_data["n"]
    m=graph_data["m"]
    x=graph_data["x"]
    y=graph_data["y"]
    edge_i=graph_data["edge_i"]
    edge_j=graph_data["edge_j"]
    #print(x[2]) works!!

    #Build Variables for Example ILP
    #GRB:INFINITY is highest possible number
    n=8
    m=4
    x=[0,2,3,4,5,5,6,6]
    y=[0,3,5,2,4,-1,6,3]
    edge_i=[0,1,2,4]
    edge_j=[6,7,3,5]

    ilp_model = Model()
    #xij==1 if edge i has color j
    x00 = ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
    x01 = ilp_model.addVar(lb=0, ub=1, obj=1, vtype=GRB.BINARY)
    x02 = ilp_model.addVar(lb=0, ub=1, obj=2, vtype=GRB.BINARY)
    x03 = ilp_model.addVar(lb=0, ub=1, obj=3, vtype=GRB.BINARY)
    x04 = ilp_model.addVar(lb=0, ub=1, obj=4, vtype=GRB.BINARY)
    x05 = ilp_model.addVar(lb=0, ub=1, obj=5, vtype=GRB.BINARY)
    x06 = ilp_model.addVar(lb=0, ub=1, obj=6, vtype=GRB.BINARY)
    x07 = ilp_model.addVar(lb=0, ub=1, obj=7, vtype=GRB.BINARY)
    x10 = ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
    x11 = ilp_model.addVar(lb=0, ub=1, obj=1, vtype=GRB.BINARY)
    x12 = ilp_model.addVar(lb=0, ub=1, obj=2, vtype=GRB.BINARY)
    x13 = ilp_model.addVar(lb=0, ub=1, obj=3, vtype=GRB.BINARY)
    x14 = ilp_model.addVar(lb=0, ub=1, obj=4, vtype=GRB.BINARY)
    x15 = ilp_model.addVar(lb=0, ub=1, obj=5, vtype=GRB.BINARY)
    x16 = ilp_model.addVar(lb=0, ub=1, obj=6, vtype=GRB.BINARY)
    x17 = ilp_model.addVar(lb=0, ub=1, obj=7, vtype=GRB.BINARY)
    x20 = ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
    x21 = ilp_model.addVar(lb=0, ub=1, obj=1, vtype=GRB.BINARY)
    x22 = ilp_model.addVar(lb=0, ub=1, obj=2, vtype=GRB.BINARY)
    x23 = ilp_model.addVar(lb=0, ub=1, obj=3, vtype=GRB.BINARY)
    x24 = ilp_model.addVar(lb=0, ub=1, obj=4, vtype=GRB.BINARY)
    x25 = ilp_model.addVar(lb=0, ub=1, obj=5, vtype=GRB.BINARY)
    x26 = ilp_model.addVar(lb=0, ub=1, obj=6, vtype=GRB.BINARY)
    x27 = ilp_model.addVar(lb=0, ub=1, obj=7, vtype=GRB.BINARY)
    x30 = ilp_model.addVar(lb=0, ub=1, obj=0, vtype=GRB.BINARY)
    x31 = ilp_model.addVar(lb=0, ub=1, obj=1, vtype=GRB.BINARY)
    x32 = ilp_model.addVar(lb=0, ub=1, obj=2, vtype=GRB.BINARY)
    x33 = ilp_model.addVar(lb=0, ub=1, obj=3, vtype=GRB.BINARY)
    x34 = ilp_model.addVar(lb=0, ub=1, obj=4, vtype=GRB.BINARY)
    x35 = ilp_model.addVar(lb=0, ub=1, obj=5, vtype=GRB.BINARY)
    x36 = ilp_model.addVar(lb=0, ub=1, obj=6, vtype=GRB.BINARY)
    x37 = ilp_model.addVar(lb=0, ub=1, obj=7, vtype=GRB.BINARY)

    #Build Constrints for ILP
    #Each edge has to have at least one color
    con0 = ilp_model.addConstr(x00+x01+x02+x03+x04+x05+x06+x07>=1)
    con1 = ilp_model.addConstr(x10+x11+x12+x13+x14+x15+x16+x17>=1)
    con2 = ilp_model.addConstr(x20+x21+x22+x23+x24+x25+x26+x27>=1)
    con3 = ilp_model.addConstr(x30+x31+x32+x33+x34+x35+x36+x37>=1)

    #For each edge i, compute cross[i]
    cross = numpy.zeros((m,m))#cross[i,j]==1 if edge i crosses edge j
    for k in range(0,m):
        for l in range(k+1,m):
            if (edge_i[k]==edge_i[l] or edge_i[k]==edge_j[l] or edge_j[k]==edge_i[l] or edge_j[k]==edge_j[l]):
                cross[k,l]=0
            elif intersect(edge_i[k], edge_j[k], edge_i[l], edge_j[l]):
                cross[k,l]=1
    print(cross)

    #if two edges cross, they need to have different colors
    for k in range(0,m):
        for l in range(k+1,m):
            if (cross[k,l]==1):
                #add the constraints as in the example below"
                cross[k,l]=1

    cross_con01_0 = ilp_model.addConstr(x00+x10<=1) #edges 0 and 1 cross, therefore they can't both have color 0
    cross_con01_1 = ilp_model.addConstr(x01+x11<=1)
    cross_con01_2 = ilp_model.addConstr(x02+x12<=1)
    cross_con01_3 = ilp_model.addConstr(x03+x13<=1)

    cross_con02_0 = ilp_model.addConstr(x00+x20<=1) #edges 0 and 2 cross, therefore they can't both have color 0
    cross_con02_1 = ilp_model.addConstr(x01+x21<=1)
    cross_con02_2 = ilp_model.addConstr(x02+x22<=1)
    cross_con02_3 = ilp_model.addConstr(x03+x23<=1)

    cross_con12_0 = ilp_model.addConstr(x10+x20<=1) #edges 1 and 2 cross, therefore they can't both have color 0
    cross_con12_1 = ilp_model.addConstr(x11+x21<=1)
    cross_con12_2 = ilp_model.addConstr(x12+x22<=1)
    cross_con12_3 = ilp_model.addConstr(x13+x23<=1)

    cross_con13_0 = ilp_model.addConstr(x10+x30<=1) #edges 1 and 3 cross, therefore they can't both have color 0
    cross_con13_1 = ilp_model.addConstr(x11+x31<=1)
    cross_con13_2 = ilp_model.addConstr(x12+x32<=1)
    cross_con13_3 = ilp_model.addConstr(x13+x33<=1)

    #Run ILP
    ilp_model.update() #adds variables and constraints to ilp_model
    ilp_model.optimize()#minimizes the objective function
    #x00.x, x01.x contain the optimal value of the variables
    #from variables, we see that edge 0 gets color 2
    #edge 1 gets color 1
    #edge 2 gets color 0
    #edge 3 gets color 0, which is correct

    #Translate ILP result into the JSON data
    #Compute edge_colors
    edge_colors = numpy.zeros(m) #edge_color[i] is the color of edge i
    used_colors = numpy.zeros(n) #if color l has already been used for at least one edge, then used_colors[l]=1
    for k in range(0,m):
        edge_colors[k]=-1
    

    #go over variables and extract the color assignments
    for k in range(0,m):
        l=0
        while(l<n and xkl.x==0):
            l=l+1
        edge_colors[k]=l
        used_colors[l]=1

    #count number of colors
    color_counter=0
    for k in range(0,n):
        if (used_colors[k]==1):
            color_counter=color_counter+1


    #Write JSON solution file
    ilp_json = {"type" : "Solution_CGSHOP2022", "instance" : "", "num_colors" : 0, "colors": []}
    ilp_json["num_colors"]=color_counter
    ilp_json["instance"]=graph_data["id"]
    ilp_json["colors"]=edge_colors

    with open('naive_ilp_solution.json', 'w') as json_file:
        json.dump(ilp_json, json_file)



if __name__ == "__main__":
    main()
