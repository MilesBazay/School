#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_TRAINS 100

char last_direction = 'W';  // start with West preference
int same_dir_count = 0;     // how many in a row in same direction

void *controller_thread (void *args);
void *train_thread (void *args);

pthread_mutex_t station_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t track_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t controller_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t east_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t west_cv = PTHREAD_COND_INITIALIZER;



typedef struct Train {
    int id;
    char direction; // E or W 
    int priority; // 1 or 0
    int load_time;
    int cross_time;
    pthread_cond_t cond;
    struct Train *next;
}Train;

Train *east_head = NULL, *west_head = NULL;
int trains_remaining = 0;

void *train_thread(void *arg);
void *controller_thread(void *arg);
void enqueue_train(Train **head, Train *train);
Train *dequeue_train(Train **head);
void print_status(const char *msg, int id, const char *dir);

int main(int argc, char *argv[]) {
    // Main function implementation
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \n", argv[0]);
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening input file");
        exit(1);
    }

    Train trains[MAX_TRAINS];
    int count = 0;
    char dir;
    int load, cross;

    while (fscanf(fp, " %c %d %d", &dir, &load, &cross) == 3) {
        trains[count].id = count;
        trains[count].direction = (dir == 'E' || dir == 'e') ? 'E' : 'W';
        trains[count].priority = (dir == 'E' || dir == 'W') ? 1 : 0;
        trains[count].load_time = load;
        trains[count].cross_time = cross;
        pthread_cond_init(&trains[count].cond, NULL);
        trains[count].next = NULL;
        count++;
    }
    fclose(fp);

    trains_remaining = count;

    pthread_t controller;
    pthread_t tids[MAX_TRAINS];

    pthread_create(&controller, NULL, controller_thread, NULL);
    for (int i = 0; i < count; i++)
        pthread_create(&tids[i], NULL, train_thread, &trains[i]);

    for (int i = 0; i < count; i++)
        pthread_join(tids[i], NULL);

    pthread_join(controller, NULL);

    return 0;

}

void *controller_thread(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&station_mutex);

        // wait for any ready trains
        while (!east_head && !west_head && trains_remaining > 0)
            pthread_cond_wait(&controller_cv, &station_mutex);

        if (trains_remaining <= 0) {
            pthread_mutex_unlock(&station_mutex);
            break;
        }

        // --- determine next train ---
        Train *next = NULL;
        char next_dir;

        // check available trains by priority
        int east_high = east_head && east_head->priority == 1;
        int west_high = west_head && west_head->priority == 1;

        if (east_high && !west_high) {
            next = dequeue_train(&east_head);
            next_dir = 'E';
        } else if (west_high && !east_high) {
            next = dequeue_train(&west_head);
            next_dir = 'W';
        } else if (east_high && west_high) {
            // both high: alternate direction
            if (last_direction == 'E') {
                next = dequeue_train(&west_head);
                next_dir = 'W';
            } else {
                next = dequeue_train(&east_head);
                next_dir = 'E';
            }
        } else {
            // both low or empty high priority queues
            if (east_head && west_head) {
                //both low: alternate direction
                if (last_direction == 'E') {
                next = dequeue_train(&west_head);
                next_dir = 'W';
                } else {
                    next = dequeue_train(&east_head);
                    next_dir = 'E';
                }
            } else if (east_head) {
                next = dequeue_train(&east_head);
                next_dir = 'E';
            } else if (west_head) {
                next = dequeue_train(&west_head);
                next_dir = 'W';
            }
        }

        if (!next) {
            pthread_mutex_unlock(&station_mutex);
            continue;
        }

        // --- signal chosen train ---
        if (next_dir == 'E')
            pthread_cond_signal(&east_cv);
        else
            pthread_cond_signal(&west_cv);

        // update scheduling history
        if (next_dir != last_direction)
            last_direction = next_dir;

        pthread_mutex_unlock(&station_mutex);

        // give time for train to cross before next iteration
        usleep(next->cross_time * 100000);
        trains_remaining--;

        // small pause to ensure output order remains clean
        usleep(20000);
    }

    return NULL;
}


void *train_thread(void *arg) {
    Train *train = (Train *)arg;

    if(train->priority == 0)
        usleep(1000); // simulate lower priority delay so that high priority trains arrive first
    usleep(train->load_time * 100000); // simulate loading
    pthread_mutex_lock(&station_mutex);
    print_status("ready", train->id, train->direction == 'E' ? "East" : "West");

    // enqueue and signal controller
    enqueue_train(train->direction == 'E' ? &east_head : &west_head, train);
    pthread_cond_signal(&controller_cv);

    // wait until controller selects this train
    if (train->direction == 'E')
        pthread_cond_wait(&east_cv, &station_mutex);
    else
        pthread_cond_wait(&west_cv, &station_mutex);
    pthread_mutex_unlock(&station_mutex);

    // simulate crossing
    pthread_mutex_lock(&track_mutex);
    print_status("ON the main track", train->id, train->direction == 'E' ? "East" : "West");
    usleep(train->cross_time * 100000);
    print_status("OFF the main track after", train->id, train->direction == 'E' ? "East" : "West");
    pthread_mutex_unlock(&track_mutex);

    return NULL;
}


void enqueue_train(Train **head, Train *train) {
    if (train == NULL) return;

    train->next = NULL;

    if (*head == NULL) {
        *head = train;
    } else {
        Train *cur = *head;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = train;
    }
}

Train *dequeue_train(Train **head) {
    if (head == NULL || *head == NULL) return NULL;

    Train *t = *head;
    *head = t->next;
    t->next = NULL;
    return t;
}

void print_status(const char *msg, int id, const char *dir) {
    pthread_mutex_lock(&print_mutex);
    
    static FILE *out = NULL;

    if (out == NULL) {
        out = fopen("output.txt", "w");
        if (out == NULL) {
            perror("Error opening output file");
            pthread_mutex_unlock(&print_mutex);
            return;
        }
    }else{
        out = fopen("output.txt", "a");
        if (!out) {
            perror("Error appending to output.txt");
            pthread_mutex_unlock(&print_mutex);
            return;
        }
    }

    if(strcmp(msg, "ready") == 0)
        fprintf(out, "Train %2d is %s to go %s\n", id, msg, dir);
    else if(strcmp(msg, "ON the main track")==0)
        fprintf(out, "Train %2d is %s going %s\n", id, msg, dir);
    else
        fprintf(out, "Train %2d is %s after going %s\n", id, msg, dir);
    
    fclose(out);

    pthread_mutex_unlock(&print_mutex);
}
