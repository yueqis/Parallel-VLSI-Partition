mpirun -n 1 ./bin/fm inputs/uniform/uniform_1000.txt outputs/output.txt
mpirun -n 2 ./bin/fm inputs/uniform/uniform_1000.txt outputs/output.txt
mpirun -n 4 ./bin/fm inputs/uniform/uniform_1000.txt outputs/output.txt
mpirun -n 8 ./bin/fm inputs/uniform/uniform_1000.txt outputs/output.txt

mpirun -n 1 ./bin/fm inputs/uniform/uniform_5000.txt outputs/output.txt
mpirun -n 2 ./bin/fm inputs/uniform/uniform_5000.txt outputs/output.txt
mpirun -n 4 ./bin/fm inputs/uniform/uniform_5000.txt outputs/output.txt
mpirun -n 8 ./bin/fm inputs/uniform/uniform_5000.txt outputs/output.txt

mpirun -n 1 ./bin/fm inputs/uniform/uniform_10000.txt outputs/output.txt
mpirun -n 2 ./bin/fm inputs/uniform/uniform_10000.txt outputs/output.txt
mpirun -n 4 ./bin/fm inputs/uniform/uniform_10000.txt outputs/output.txt
mpirun -n 8 ./bin/fm inputs/uniform/uniform_10000.txt outputs/output.txt

mpirun -n 1 ./bin/fm inputs/uniform/uniform_100000.txt outputs/output.txt
mpirun -n 2 ./bin/fm inputs/uniform/uniform_100000.txt outputs/output.txt
mpirun -n 4 ./bin/fm inputs/uniform/uniform_100000.txt outputs/output.txt
mpirun -n 8 ./bin/fm inputs/uniform/uniform_100000.txt outputs/output.txt

mpirun -n 1 ./bin/fm inputs/uniform/uniform_200000.txt outputs/output.txt
mpirun -n 2 ./bin/fm inputs/uniform/uniform_200000.txt outputs/output.txt
mpirun -n 4 ./bin/fm inputs/uniform/uniform_200000.txt outputs/output.txt
mpirun -n 8 ./bin/fm inputs/uniform/uniform_200000.txt outputs/output.txt