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
    #for index in range(len(arg_T)):
        #print(arg_T[index])
    x=arg_X[8]     #分别生成三个维度数据
    y=arg_Y[8]
    t=arg_T[8]
    ax.plot(x, y, t, 'go--')
    plt.show()
    return arg_X

