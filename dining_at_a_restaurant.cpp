#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

sem_t cooking_req_mutex;
sem_t cooking_done_mutex;
sem_t request_dish_mutex;
pthread_barrier_t eating_barrier;

int totalDishOrdered;

bool isHead() {

        int randNumber = rand(); 
        if (randNumber % 2 == 0) return true;
        else return false;
}

void *cookCuisines(void *arg) {


	for (int i = 0; i < totalDishOrdered; i++) {

		sem_wait(&cooking_req_mutex);

		if (isHead()) {
                        sleep(5);
                }

		printf("\t\tChef: dish %d is ready, you can take it to the guests.\n", i + 1);

		sem_post(&cooking_done_mutex);
	}

	return NULL;
}

void *serveGuests(void *arg) {

	printf("Waiter: Sirs, the serving will be started soon.\n");

	for (int i = 0; i < totalDishOrdered; i++) {
	
		sem_wait(&request_dish_mutex);

		printf("\tWaiter: Chef, the guests are ready for dish no %d!\n", i + 1);

		sem_post(&cooking_req_mutex);
		
		sem_wait(&cooking_done_mutex);
		
		if (isHead()) {
                        sleep(5);
                }

		printf("\t\t\tWaiter: Sirs, this is your dish %d. Enjoy!\n", i + 1);
		
		pthread_barrier_wait(&eating_barrier);
		pthread_barrier_wait(&eating_barrier);

	}
	
	return NULL;
}

void *eat(void *arg) {

	int *intArg = (int *) arg;
	int guestNo = *intArg + 1;

	for (int i = 0; i < totalDishOrdered; i++) {

		if (guestNo == 1) {
			printf("\n\nGuest %d: Waiter, we are ready for dish %d\n", guestNo, i + 1);
			sem_post(&request_dish_mutex);
		}
	
		pthread_barrier_wait(&eating_barrier);

		if (isHead()) {
                        sleep(5);
                }

		printf("\t\t\t\tGuest %d: I am done eating dish %d, what a delicious dish!\n", guestNo, i + 1);
		pthread_barrier_wait(&eating_barrier);
	}

	return NULL;
}



int main(int argc, char *argv[]) {

	pthread_t *threads;
	int *guestParams;

	if (argc != 3) {
                printf("You need to pass two command line argument to properly use this program.\n");
                printf("Usage: %s n m\n", argv[0]);
                printf("Here n is number of guests and m the number of food items they will eat.\n");
                exit(1);
        }

	int guestCount = atoi(argv[1]);
	totalDishOrdered = atoi(argv[2]);

	printf("Main: total guests %d, total dishes %d\n", guestCount, totalDishOrdered);

	srand(time(NULL));
	sem_init(&cooking_done_mutex, 0, 0);
	sem_init(&cooking_req_mutex, 0, 0);
	sem_init(&request_dish_mutex, 0, 0);
	pthread_barrier_init(&eating_barrier, NULL, guestCount + 1);
	
        threads = (pthread_t *) malloc((guestCount + 2) * sizeof(*threads));
        guestParams = (int *) malloc(sizeof(int) * guestCount);

	for (int i = 0; i < guestCount; i++) {
		guestParams[i] = i;
		pthread_create(&threads[i], NULL, eat, (void *) &guestParams[i]);
	}
	
	pthread_create(&threads[guestCount], NULL, serveGuests, NULL);
	pthread_create(&threads[guestCount + 1], NULL, cookCuisines, NULL);

	for (int i = 0; i < guestCount + 1; i++) {
                pthread_join(threads[i], NULL);
        }

	printf("Main: the program is terminating.\n");

    	sem_destroy(&cooking_req_mutex);
    	sem_destroy(&cooking_done_mutex);
    	sem_destroy(&request_dish_mutex);
    	pthread_barrier_destroy(&eating_barrier);

	free(threads);
	free(guestParams);

	return 0;
}
