import numpy as np
import glob

for learner in glob.glob("/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/learners_flattened/*.npz"):
    data = np.load(learner)
    print(learner)

    arr_0 = data["arr_0"]
    arr_1 = data["arr_1"]
    arr_2 = data["arr_2"]
    arr_3 = data["arr_3"]
    arr_4 = data["arr_4"]
    arr_5 = data["arr_5"]
    arr_6 = data["arr_6"]
    arr_7 = data["arr_7"]
    arr_8 = data["arr_8"]
    arr_9 = data["arr_9"]
    arr_10 = data["arr_10"]
    arr_11 = data["arr_11"]
    arr_12 = data["arr_12"]
    arr_13 = data["arr_13"]


    arr_0 = arr_0.flatten()
    arr_1 = arr_1.flatten()
    arr_2 = arr_2.flatten()
    arr_3 = arr_3.flatten()
    arr_4 = arr_4.flatten()
    arr_5 = arr_5.flatten()
    arr_6 = arr_6.flatten()
    arr_7 = arr_7.flatten()
    arr_8 = arr_8.flatten()
    arr_9 = arr_9.flatten()
    arr_10 = arr_10.flatten()
    arr_11 = arr_11.flatten()
    arr_12 = arr_12.flatten()
    arr_13 = arr_13.flatten()


    np.savez(learner, arr_0=arr_0, arr_1=arr_1, arr_2=arr_2, arr_3=arr_3, arr_4=arr_4, arr_5=arr_5, arr_6=arr_6, arr_7=arr_7, arr_8=arr_8, arr_9=arr_9, arr_10=arr_10, arr_11=arr_11, arr_12=arr_12, arr_13=arr_13)
