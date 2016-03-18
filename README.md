# ParallelFireSimulator
This project consists in the parallelization of a fire simulator. The followed strategy is a master-workers combined with multithread computation inside each node. The parallelization consists on testing every possible combination of parameters during simulation (model, wind velocity, etc.).
For master-workers communications is employed the MPI library, whereas for the multithread computation is used the Pthread library.
