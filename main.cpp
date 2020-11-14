#include <mpi.h>
#include <omp.h>
#include <getopt.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>

// TODO change spacial parameters type to float with corresponding validation changes
int param_x_bound = 100;
int param_y_bound = 100;
int param_z_bound = 100;
int param_t_bound = 10;
// steps number
int param_steps = 100;
// threads number 
int param_nt = 1;

// total number of processes involved and current process id
int nproc, rank;
const int MASTER_PROCESS = 0;

typedef enum {
    INVALID_PARAMETERS = 1
} errcode;


bool validate_parameters() {

    bool is_valid = true;
    if (param_x_bound <= 0) {
        std::cerr << "input parameters error:"
            " --xbound (-x) must be positive integer, but " <<
            param_x_bound << " was given" << std::endl;
        is_valid = false;
    }

    if (param_y_bound <= 0) {
        std::cerr << "input parameters error:"
            " --ybound (-y) must be positive integer, but " <<
            param_y_bound << " was given" << std::endl;
        is_valid = false;
    }

    if (param_z_bound <= 0) {
        std::cerr << "input parameters error:"
            " --zbound (-z) must be positive integer, but " <<
            param_z_bound << " was given" << std::endl;
        is_valid = false;
    }

    if (param_t_bound <= 0) {
        std::cerr << "input parameters error:"
            " --tbound (-t) must be positive integer, but " <<
            param_t_bound << " was given" << std::endl;
        is_valid = false;
    }

    if (param_steps <= 0) {
        std::cerr << "input parameters error:"
            " --steps (-s) must be positive integer, but " <<
            param_steps << " was given" << std::endl;
        is_valid = false;
    }

    if (param_nt <= 0) {
        std::cerr << "input parameters error:"
            " --nt (-n) must be positive integer, but " <<
            param_nt << " was given" << std::endl;
        is_valid = false;
    }

    return is_valid;
}

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // array for parameters transition from the master to slaves
    int params_storage[4];

    // processing and validating input values
    if (rank == MASTER_PROCESS) {

        int option_char = -1, 
            option_idx = 0;
        std::string help_message = 
            "Usage: <binary> [OPTIONS], where OPTIONS include\n"
            "[--help | -h] \n"
            "[--xbound=<upper X boundary for computational grid> | -x <...>] \n"
            "[--ybound=<upper Y boundary for computational grid> | -y <...>] \n"
            "[--zbound=<upper Z boundary for computational grid> | -z <...>] \n"
            "[--tbound=<temporal upper limit for the model> | -t <...>] \n" 
            "[--steps=<number of iterations to perform> | -s <...>] \n"
            "[--nt=<thread number> | -n <...>] \n";

        while (true) {

            static struct option long_options[] = {
                { "xbound", required_argument, 0, 'x' }, 
                { "ybound", required_argument, 0, 'y' },
                { "zbound", required_argument, 0, 'z' },
                { "tbound", required_argument, 0, 't' },
                { "steps",  required_argument, 0, 's' },
                { "nt",     required_argument, 0, 'n' }, 
                { "help",   no_argument,       0, 'h' },
                { 0,        0,                 0,  0  }
            };

            option_char = getopt_long(argc, argv, "x:y:z:t:s:n:h", 
                long_options, &option_idx);
            if (option_char == -1) {
                break;
            }

            switch (option_char) {
                case 'x':
                    param_x_bound = atoi(optarg);
                    break;

                case 'y':
                    param_y_bound = atoi(optarg);
                    break;

                case 'z':
                    param_z_bound = atoi(optarg);
                    break;

                case 't':
                    param_t_bound = atoi(optarg);
                    break;

                case 's':
                    param_steps = atoi(optarg);
                    break;

                case 'n':
                    param_nt = atoi(optarg);
                    break;

                case 'h':
                    std::cout << help_message << std::endl;
                    break;

                case '?':
                    break;
            }
        }

        if (argc == 1) {
            std::cout << help_message << std::endl;
        }

        if (!validate_parameters()) {
            MPI_Abort(MPI_COMM_WORLD, INVALID_PARAMETERS);
            exit(INVALID_PARAMETERS);
        }

        params_storage[0] = param_x_bound;
        params_storage[1] = param_y_bound;
        params_storage[2] = param_z_bound;
        params_storage[3] = param_nt;
    }

    // receiving parameters from MASTER
    // TODO beware of param_XXX_bound type changes as MPI_INT is strictly set
    MPI_Bcast(params_storage, 4, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    param_x_bound = params_storage[0];
    param_y_bound = params_storage[1];
    param_z_bound = params_storage[2];
    param_nt = params_storage[3];

    // <test_code>
    std::cout << "x=" << param_x_bound << std::endl <<
                 "y=" << param_y_bound << std::endl <<
                 "z=" << param_z_bound << std::endl <<
                 "threads=" << param_nt << std::endl <<
                 "in rank #" << rank << std::endl;

    std::cout << "settings threads number to " << param_nt << " in process #" <<
        rank << std::endl;
    omp_set_num_threads(param_nt);

    int accum = 0;
    #pragma omp parallel for reduction(+: accum)
    for (int i = 0; i < param_x_bound * param_y_bound * param_z_bound; ++i) {
        if (i % 2) {
            accum += i;
        } else {
            accum -= i;
        }

    }

    std::cout << "accumulator = " << accum << std::endl;
    // </test_code>

    MPI_Finalize();
    exit(0);
}
