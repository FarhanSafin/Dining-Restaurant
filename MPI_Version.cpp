#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define REQUEST_DISH_TAG 1
#define COOKING_REQ_TAG 2
#define COOKING_DONE_TAG 3

int totalDishOrdered;
bool isHead() {
    int randNumber = rand();
    if (randNumber % 2 == 0)
        return true;
    else
        return false;
}

void cookCuisines(int world_rank, int world_size) {
    int dishCount = 0;
    while (dishCount < totalDishOrdered) {
        MPI_Recv(NULL, 0, MPI_INT, 0, COOKING_REQ_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (isHead()) {
            sleep(5);
        }
        printf("\t\tChef %d: dish %d is ready, you can take it to the guests.\n", world_rank, dishCount + 1);
        MPI_Send(NULL, 0, MPI_INT, 0, COOKING_DONE_TAG, MPI_COMM_WORLD);
        dishCount++;
    }
}

void serveGuests(int world_rank, int world_size) {
    printf("Waiter %d: Sirs, the serving will be started soon.\n", world_rank);
    for (int i = 0; i < totalDishOrdered; i++) {
        MPI_Send(NULL, 0, MPI_INT, 1, REQUEST_DISH_TAG, MPI_COMM_WORLD);
        printf("\tWaiter %d: Chef, the guests are ready for dish no %d!\n", world_rank, i + 1);
        MPI_Recv(NULL, 0, MPI_INT, 1, COOKING_REQ_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (isHead()) {
            sleep(5);
        }
        printf("\t\t\tWaiter %d: Sirs, this is your dish %d. Enjoy!\n", world_rank, i + 1);
        MPI_Send(NULL, 0, MPI_INT, 0, COOKING_DONE_TAG, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void eat(int world_rank, int world_size) {
    int guestNo = world_rank - 2;
    for (int i = 0; i < totalDishOrdered; i++) {
        if (guestNo == 0) {
            printf("\n\nGuest %d: Waiter, we are ready for dish %d\n", guestNo + 1, i + 1);
            MPI_Recv(NULL, 0, MPI_INT, 1, REQUEST_DISH_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (isHead()) {
            sleep(5);
        }
        printf("\t\t\t\tGuest %d: I am done eating dish %d, what a delicious dish!\n", guestNo + 1, i + 1);
        MPI_Send(NULL, 0, MPI_INT, 1, COOKING_REQ_TAG, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    int world_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(time(NULL) + world_rank);

    if (world_rank == 0) { // chef process
        totalDishOrdered = world_size - 2;
        cookCuisines(world_rank, world_size);
    } else if (world_rank == 1) { // waiter process
        serveGuests(world_rank, world_size);
    } else { // guest processes
        eat(world_rank, world_size);
    }

    MPI_Finalize();
    return 0;
}