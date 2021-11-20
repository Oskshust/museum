// Coded by: Oskar Szudzik, AI, 148245

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

// declaring semaphores, basic numbers and probability

sem_t semaphoreA; // tells us how many more guests we can fit into hall A
sem_t semaphoreB; // tells us how many more guests we can fit into hall A
sem_t leftA; // both leftA and leftB semaphores prevent an overflow of guests
sem_t leftB;

int maxA = 150;  // max number of people in hall A
int maxB = 100;  // max number of people in hall B
int num_of_guests = 500; // num of people in a queue
int enter_b = 80;    // prob of entering hall B

void print_stats(){
    // if temp is positive -> it tells us how many guests can enter
    // if temp is negative -> it tells us how many threads
    // are waiting for this semaphore
    int temp;
    sem_getvalue(&semaphoreA, &temp);
    printf("People in hall A: %d\n", temp);
    sem_getvalue(&semaphoreB, &temp);
    printf("People in hall B: %d\n", temp);
}

void* guest(){ // function that mimic typical visitor's behaviour
    // guest wants to enter the museum
    // check if there is enough room in hall A
    sem_wait(&semaphoreA);
    // I used sleep function to better show statistics
    // which prove that program works
    sleep(5);
    // check if object wants to enter hall B, check room
    srand(time(NULL));
    if (rand()%100 <= enter_b){
        sem_wait(&semaphoreB);
        sem_post(&leftA);
        sleep(5);
        // guest wants to leave, check room in A
        sem_wait(&semaphoreA);
        sem_post(&leftB);
        sem_post(&leftA);
    }
    else{
        sleep(5);
        sem_post(&leftA);
    }
}

void* gatekeeperA(){
    // gatekeeper waits until someone wants to leave hall A
    while(1) {
        sem_wait(&leftA);
        sem_post(&semaphoreA);
        // I decided to print statistics here, because now we can observe queue
        // to both hall A and hall B
        print_stats();
    }
}

void* gatekeeperB(){
    // gatekeeper waits until someone wants to leave hall B
    while(1) {
        sem_wait(&leftB);
        sem_post(&semaphoreB);
    }
}

int main() {
    // initializing semaphores
    if (sem_init(&semaphoreA, 0, maxA) != 0) {
        printf("\n semaphore init has failed\n");
        return 1;
    }
    if (sem_init(&semaphoreB, 0, maxB) != 0) {
        printf("\n semaphore init has failed\n");
        return 1;
    }
    if (sem_init(&leftA, 0, 0) != 0) {
        printf("\n semaphore init has failed\n");
        return 1;
    }
    if (sem_init(&leftB, 0, 0) != 0) {
        printf("\n semaphore init has failed\n");
        return 1;
    }

    // creating threads - gatekeepers + guests
    pthread_t people[num_of_guests];
    pthread_t gateA;
    pthread_t gateB;

    pthread_create(&gateA, NULL, &gatekeeperA, NULL);
    pthread_create(&gateB, NULL, &gatekeeperB, NULL);
    for(int i = 0; i < num_of_guests; i++) {
        pthread_create(&people[i], NULL, &guest, NULL);
        printf("%d in\n", i);
        //print_stats();
        // Printing statistics here can be misleading in some part.
        // Numbers for queue to hall A are very logical - firstly we see that
        // more and more guests are coming in, later a lot of them are waiting
        // However, because of high sleep() values, we can not observe
        // a migration to hall B.
    }
    // joining threads
    for(int i = 0; i < num_of_guests; i++){
        pthread_join(people[i], NULL);
    }
    pthread_join(gateA, NULL);
    pthread_join(gateB, NULL);

    // destroying semaphores
    sem_destroy(&semaphoreA);
    sem_destroy(&semaphoreB);
    return 0;
}