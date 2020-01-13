import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import random
import matplotlib as mpl
from matplotlib import cm
from mpl_toolkits.mplot3d import axes3d
from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection

def plot_3d(arg_X,arg_Y,arg_T) :
    print("[DONE] call python success!")
    fig = plt.figure()
    ax = Axes3D(fig)
    #ax = fig.gca(projection='3d')
    ax.set_title("3D_trajectory")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("T")
    for index in range(len(arg_T)):
        print(arg_T[index])
        x=arg_X[index]     #分别生成三个维度数据
        y=arg_Y[index]
        t=arg_T[index]
        ax.plot(x, y, t, 'bo--')
    #x2=arg_X[0]     #分别生成三个维度数据
    #y2=arg_Y[0]
    #t2=arg_T[0]
    #ax.plot(x2, y2, t2, 'bo--')
    plt.show()
    return arg_X

