#include "user.h"

#define T_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            exit(1); \
        } \
    } while (0)


int N = 1000;

void sem_basic() {
    printf("[sem_basic] Initializing semaphore to 1...\n");
    int semid = sem_init(1);
    T_ASSERT(sem_getvalue(semid) == 1);

    printf("[sem_basic] Waiting (should decrement to 0)...\n");
    sem_wait(semid);
    T_ASSERT(sem_getvalue(semid) == 0);

    printf("[sem_basic] Posting (should increment to 1)...\n");
    sem_signal(semid);
    T_ASSERT(sem_getvalue(semid) == 1);

    sem_free(semid);
    printf("[sem_basic] PASS: Basic semaphore operations work.\n");
}

void do_work(int pid, int semid) {
    for (int i = 0; i < N; i++) {
        // TODO: increment a shared counter, making sure to use avoid race condition.
        // Tip: use a semaphore.
        //sem_wait(semid);
        T_ASSERT(sem_wait(semid) == 0); // Same as above just that we check
        int val = ucnt_get(0); // Read
        ucnt_set(0, val + 1); //Write
        //sem_signal(semid);
        T_ASSERT(sem_signal(semid) == 0);
    }
}

void race_test() {
    ucnt_set(0, 0);
    // TODO: create semaphore properly
    int semid = sem_init(1);       // was: int semid = -1;
    T_ASSERT(semid >= 0); //T_ASSERT is just an assert function ton ensure run as expected

    int pid = fork();
    T_ASSERT(pid >= 0);
    if (pid == 0) { // child
        do_work(pid, semid);
        exit(0);
    } else { // parent
        do_work(getpid(), semid);
        wait(0); //Checkpoint 4: wait before validating and free sephomore? sem_free only makrs slot unused/ doesnt wait for child or wake anyone
        int final = ucnt_get(0); 
        printf("[race_test] final counter=%d (expected %d)\n", final, 2*N);
        T_ASSERT(final == 2*N);
        printf("[race_test] PASS: consistent counter\n");
    }

    sem_free(semid);
}    

struct buf_sem {
    // TODO: add semaphores as required
    int free_semid;
    int full_semid;
    int empty_semid;
};

void consumer(struct buf_sem b, int loops, int valid[]) {
    char tmp;
    for(int i = 0; i < loops; i++) {
        // TODO: wait buffer slot full and signal empty slot 
        sem_wait(b.full_semid); // wait if not full (value = 0), start consume once full aka > 0
        sem_wait(b.free_semid);
        tmp = ubuf_read();
        T_ASSERT(valid[(unsigned char)tmp]);
        sem_signal(b.free_semid);
        sem_signal(b.empty_semid); // increment and tell others it is not empty 
    }
}

void producer(const char* msg, struct buf_sem b) {
    for (const char* p = msg; *p != '\0'; p++) {
        // TODO: wait buffer slot emtpy and signal used slot
        sem_wait(b.empty_semid); // wait if not empty (value = 0), start produce once empty aka > 0
        sem_wait(b.free_semid);
        ubuf_write(*p);
        sem_signal(b.free_semid);
        sem_signal(b.full_semid); //increment and tell others it is full
    }
}

void producer_consumer() {
    #define MSG_SIZE 48
    const char msg[MSG_SIZE + 1] = "cafefacefacefacefacefacefacefacefacecafecafecafe";
    const int NUM_PROD = 2;
    int valid[256] = {0};
    for (const char* p = msg; *p != '\0'; p++) {
        unsigned char uc = (unsigned char)*p;
        valid[uc] = 1;
    }

    struct buf_sem b;
    // TODO init semaphores as required
    b.empty_semid = sem_init(UBUF_SIZE); //since we set UBUF_SIZE = 16, producer can straight away start to produce while consumer waits
    b.full_semid = sem_init(0);
    b.free_semid = sem_init(1);

    for (int i = 0; i < NUM_PROD; i++) {
        int pid = fork();
        T_ASSERT(pid >= 0);
        if (pid == 0) { // child
            producer(msg, b);
            exit(0);
        }
    }

    consumer(b, MSG_SIZE * NUM_PROD, valid);
    for (int i = 0; i < NUM_PROD; i++) {
        wait(0);
    }
    printf("[producer_consumer] PASS: message delivered\n");
    // TODO clean-up semaphores
    //Note sem_free sets in_use to 0 it doesnt wait for/wake anyone > run at the end
    sem_free(b.empty_semid);             // now safe: nobody uses them anymore
    sem_free(b.full_semid);
    sem_free(b.free_semid);
}



int main() {
    sem_basic();
    
    race_test();

    producer_consumer();

    printf("ALL TESTS PASSED\n");

    exit(0);
}
