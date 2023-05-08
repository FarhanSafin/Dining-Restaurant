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

void cookCuisines(int rank, int size) {
    int dishCount = 0;
    while (dishCount < totalDishOrdered) {
        MPI_Recv(NULL, 0, MPI_INT, 0, COOKING_REQ_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (isHead()) {
            sleep(5);
        }
        printf("\t\tChef %d: dish %d is ready, you can take it to the guests.\n", rank, dishCount + 1);
        MPI_Send(NULL, 0, MPI_INT, 0, COOKING_DONE_TAG, MPI_COMM_WORLD);
        dishCount++;
    }
}

void serveGuests(int rank, int size) {
    printf("Waiter %d: Sirs, the serving will be started soon.\n", rank);
    for (int i = 0; i < totalDishOrdered; i++) {
        MPI_Send(NULL, 0, MPI_INT, 1, REQUEST_DISH_TAG, MPI_COMM_WORLD);
        printf("\tWaiter %d: Chef, the guests are ready for dish no %d!\n", rank, i + 1);
        MPI_Recv(NULL, 0, MPI_INT, 1, COOKING_REQ_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (isHead()) {
            sleep(5);
        }
        printf("\t\t\tWaiter %d: Sirs, this is your dish %d. Enjoy!\n", rank, i + 1);
        MPI_Send(NULL, 0, MPI_INT, 0, COOKING_DONE_TAG, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void eat(int rank, int size) {
    int guestNo = rank - 2;
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
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank);

    if (rank == 0) {
        totalDishOrdered = size - 2;
        cookCuisines(rank, size);
    } else if (rank == 1) {
        serveGuests(rank, size);
    } else {
        eat(rank, size);
    }

    MPI_Finalize();
    return 0;
}