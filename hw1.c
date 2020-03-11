#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAXCHAR 10

float* ReadInputFile(char* input_file_path, int number_of_lines) {
    FILE *fp;
    char *line = malloc(MAXCHAR*sizeof(char));
    size_t index = 0;
    float line_value;
    fp = fopen(input_file_path, "r");
    if(fp==NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    float* data_array = malloc(sizeof(float)*number_of_lines);
    while((fgets(line,MAXCHAR,fp)) != NULL) {
        line_value = strtof(line, NULL);
        data_array[index++] = line_value;
        if(index==number_of_lines) break;
    }
    return data_array;
}

float findMax(float* array, size_t array_size) {
    float max = array[0];
    for(size_t i=1; i<array_size; i++) {
        if(array[i] > max)
            max = array[i];
    }
    return max;
}

void printResults(float max, float time) {
    printf("%f\n", max);
    printf("%f\n", time);
}

int main(int argc, char **argv){
    char* input_file_path = argv[1];
    int number_of_lines = atoi(argv[2]);
    int process_rank, process_count, sub_array_length;
    double start_time, end_time;
    float local_max, received_max;
    float* array = ReadInputFile(input_file_path, number_of_lines);
    int array_size = number_of_lines;
    float* sub_array = malloc(array_size*sizeof(float));
    int alive = 1;
    MPI_Status status;

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    if(process_rank == 0) {
        // 0th process works as master process
        // Get the number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &process_count);
        if(process_count == 1) {
            start_time = MPI_Wtime();
            // If there is only one process is created then the program runs in sequential
            local_max = findMax(array, number_of_lines);
            end_time = MPI_Wtime();
            printResults(local_max, end_time-start_time);
        }  else {
            // If there is more than 1 process
            sub_array_length = array_size / process_count;
            int i;
            for(i=1;i<process_count;i++) {
                MPI_Send(&sub_array_length,1,MPI_INT,i,0,MPI_COMM_WORLD);
                // Send the part of the array to process i
                MPI_Send(array+i*sub_array_length,sub_array_length, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            }
            start_time = MPI_Wtime();
            local_max = findMax(array, sub_array_length);
            /* 1st turn of value sharing */
            if(process_count >= 2) {
                MPI_Recv(&received_max,1,MPI_FLOAT,1,0, MPI_COMM_WORLD, &status);
                //printf("1st round %d got data from %d %f\n", process_rank, (process_rank+1), received_max);
                if(received_max > local_max) local_max = received_max;
            }
            /* 2nd turn of value sharing */
            if(process_count >= 4) {
                MPI_Recv(&received_max,1,MPI_FLOAT,2,0, MPI_COMM_WORLD, &status);
                //printf("1st round %d got data from %d %f\n", process_rank, (process_rank+2), received_max);
                if(received_max > local_max) local_max = received_max;
            }
            /* 3rd turn of value sharing */
            if(process_count >= 8) {
                MPI_Recv(&received_max,1,MPI_FLOAT,4,0, MPI_COMM_WORLD, &status);
                if(received_max > local_max) local_max = received_max;
            }
            /* 4th turn of value sharing */
            if(process_count >= 16) {
                MPI_Recv(&received_max,1,MPI_FLOAT,8,0, MPI_COMM_WORLD, &status);
                if(received_max > local_max) local_max = received_max;
            }
            end_time = MPI_Wtime();
            printResults(local_max, end_time-start_time);
        }
    } else {
        /* Get the process id of process */
        MPI_Comm_size(MPI_COMM_WORLD, &process_count);
        /* Get the length of sub array*/
        MPI_Recv(&sub_array_length,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		/* And receive the subarray */
        MPI_Recv(sub_array,sub_array_length,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        /* Calculate the max number in given array */
        local_max = findMax(sub_array,sub_array_length);
        /* 
            After a process send it's local max, it's alive val becomes 0 exists.
            
            First turn begins, one of every two process sends the local max to their pairs 
            1->0, 3->2 5->4...
        */
        if (alive && process_count >= 2 && process_rank % 2 == 1) {
            //printf("1st round %d sending data to %d\n", process_rank, (process_rank-1));
            //fflush(stdout);
            MPI_Send(&local_max,1,MPI_FLOAT,process_rank-1,0,MPI_COMM_WORLD);
            /* Kill the process after sending local maximum */
            alive = 0;
        } 
        if(alive && process_count >= 2 && process_rank % 2 == 0) {
            /* Get the local max of the pair process */
            //printf("1st round %d got data from %d\n", process_rank, (process_rank+1));
            //fflush(stdout);
            MPI_Recv(&received_max,1,MPI_FLOAT,process_rank+1,0, MPI_COMM_WORLD, &status);
            if(received_max > local_max) local_max = received_max;
        }
        /* 
            Second turn begins, one of every two process sends the local max to their pairs 
            2->0 6->4 10->8 14->12
        */
        if (alive && process_count >= 4 && process_rank % 4 == 2) {
            //printf("2nd round %d sending data to %d\n", process_rank, (process_rank-2));
            //fflush(stdout);
            MPI_Send(&local_max,1,MPI_FLOAT,process_rank-2,0,MPI_COMM_WORLD);
            /* Kill the process after sending local maximum */
            alive = 0;
        } 
        if(alive && process_count >= 4 && process_rank % 4 == 0) {
            /* Get the local max of the pair process */
            //printf("2nd round %d got data from %d\n", process_rank, (process_rank+2));
            //fflush(stdout);
            MPI_Recv(&received_max,1,MPI_FLOAT,process_rank+2,0, MPI_COMM_WORLD, &status);
            if(received_max > local_max) local_max = received_max;
        }
        /* 
            Third turn begins, one of every two process sends the local max to their pairs 
            4->0, 12->8
        */
        if (alive && process_count >= 8 && process_rank % 8 == 4) {
            //printf("3rd round %d sending data to %d\n", process_rank, (process_rank-4));
            //fflush(stdout);
            MPI_Send(&local_max,1,MPI_FLOAT,process_rank-4,0,MPI_COMM_WORLD);
            /* Kill the process after sending local maximum */
            alive = 0;
        } 
        if(alive && process_count >= 8 && process_rank % 8 == 0) {
            /* Get the local max of the pair process */
            //printf("3rd round %d got data from %d\n", process_rank, (process_rank+4));
            //fflush(stdout);
            MPI_Recv(&received_max,1,MPI_FLOAT,process_rank+4,0, MPI_COMM_WORLD, &status);
            if(received_max > local_max) local_max = received_max;
        }
        /* 
            Fourth turn begins, one of every two process sends the local max to their pairs 
            8->0
        */
        if (alive && process_count >= 16 && process_rank % 16 == 8) {
            //printf("4th round %d sending data to %d\n", process_rank, (process_rank-8));
            //fflush(stdout);
            MPI_Send(&local_max,1,MPI_FLOAT,process_rank-8,0,MPI_COMM_WORLD);
            alive = 0;
        } 
        if(alive && process_count >= 16 && process_rank % 16 == 0) {
            /* Get the local max of the pair process */
            //printf("4th round %d got data from %d\n", process_rank, (process_rank+8));
            //fflush(stdout);
            MPI_Recv(&received_max,1,MPI_FLOAT,process_rank+8,0, MPI_COMM_WORLD, &status);
            if(received_max > local_max) local_max = received_max;
        }
        //printf("%d process exiting \n", process_rank);
    }
    MPI_Finalize();
    return 0;
}