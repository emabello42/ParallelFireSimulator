# ParallelFireSimulator
This work shows the parallelization of a fire simulator. The followed strategy is a master-workers combined with multithread computation inside each node. The parallelization consists on testing every possible combination of parameters during simulation (model, wind velocity, etc.).
For master-works communications is employed the MPI library, where as for the multithread computation is used the Pthread library.
