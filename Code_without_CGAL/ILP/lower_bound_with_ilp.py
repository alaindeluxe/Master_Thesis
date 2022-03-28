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
    greedy_colors=24
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
    for k in range(0,m):
        for l in range(k+1,m):
            if (edge_i[k]==edge_i[l] or edge_i[k]==edge_j[l] or edge_j[k]==edge_i[l] or edge_j[k]==edge_j[l]):
                cross[k,l]=0
            elif intersect(edge_i[k], edge_j[k], edge_i[l], edge_j[l]):
                cross[k,l]=1
            print("computed intersection for ",k,l)

    flag=0
    while (flag==0):
        ilp_model = Model()
        #xi==j if edge i has color j
        variables = {}
        for k in range(0,m):
            variables[k]=ilp_model.addVar(lb=0, ub=greedy_colors-1, obj=0, vtype=GRB.INTEGER)
            print("added variable", k)
        ilp_model.update()

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
                    #print("added unequal constraint for ",k,l)
                    print(variables[0].ub)
        ilp_model.update()

        #Run ILP
        ilp_model.Params.TimeLimit = 300 #seconds
        ilp_model.update()
        print("Current lower bound = ", greedy_colors)
        ilp_model.optimize() #minimizes the objective function
        if (ilp_model.status==GRB.Status.TIME_LIMIT):
            flag=1
        elif (ilp_model.status==GRB.Status.INFEASIBLE):
            greedy_colors=greedy_colors+1
        else:
            flag=1

    print("-----------------------")
    print("-----------------------")

    print("For the graph", graph_data["id"])
    print("We determined the following lower bound on the number of colors:", greedy_colors)


if __name__ == "__main__":
    main()
