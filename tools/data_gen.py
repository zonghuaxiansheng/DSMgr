import os
import sys
import random

f = open("../data/db-data-gen.txt", "w")

for i in range(0, 2000):
    ops_e_ = random.randint(0,1)
    page_id_ = random.randint(0,200)
    f.write(str(ops_e_) + "," + str(page_id_) + "\n")

f.close()
