import sys
import random
 
# total arguments
cellNum = int(sys.argv[1])
netNum = int(sys.argv[2])

filename = "inputs/uniform/uniform_"+str(cellNum)+".txt"
file1 = open(filename, "w")  # write mode
file1.write(str(cellNum)+" "+str(netNum)+"\n")
for i in range(netNum):
    c1, c2 = random.randint(1, cellNum), random.randint(1, cellNum)
    while c1 == c2:
        c1, c2 = random.randint(1, cellNum), random.randint(1, cellNum)
    file1.write(str(c1)+" "+str(c2)+"\n")
file1.close()
