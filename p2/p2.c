#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_SIZE 30

struct customer_info{ 
    int user_id;
	int class_type;
	int service_time;
	int arrival_time;
} customer_info;

int economic_size,business_size, business_q_length, econ_q_length, count = 0;
int economic[MAX_SIZE];
int business[MAX_SIZE];
int clerk_arr[5];

// 0 and 1 next to variable's name represent 
// economy and business class respectively
pthread_mutex_t start_time_mutex, mutex0, mutex1;
pthread_cond_t cond0, cond1, clerk0, clerk1;
pthread_mutex_t clerk_mutex0, clerk_mutex1;

struct timeval start_time;
double econ_count, business_count;
double overall_waiting_time, economy_waiting_time, business_waiting_time;

int econ_q_status, business_q_status, winner_econ, winner_business = 0;


// provided on Course brightspace website
double getCurrentSimulationTime(){
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	pthread_mutex_lock(&start_time_mutex);
	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
	pthread_mutex_unlock(&start_time_mutex);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}

// enqueues and increment queue's length
void enqueue(int customerID, int class) {
    if (class==0) {
        economic[econ_q_length] = customerID;
        econ_q_length++;
    }
    else if (class==1) {
        business[business_q_length] = customerID;
        business_q_length++;
    }   
}

// dequeues and decrement queue's length
void dequeue(int class) {
    if (class==0) {
        for (int i=1; i<econ_q_length; i++) {
            economic[i-1] = economic[i];
        }
        econ_q_length--;
    } else if (class==1) {
        for (int i=1; i<business_q_length; i++) {
            business[i-1] = business[i];
        }
        business_q_length--;
    }
   
}

// returns the first element of a queue
int get(int class) {
    if (class==0) {
        return economic[0];
    }
    else if (class==1) {
        return business[0];
    }
    return 0;
}

// function for clerk threads
void *clerk_function(void *arg) {
    int * clerk_id = (int *)arg;
    while (1) {
        if (business_q_length>0) {
            pthread_mutex_lock(&mutex1);
            business_q_status = *clerk_id;
            pthread_cond_broadcast(&cond1);
            winner_business = 0;
            pthread_mutex_unlock(&mutex1);
            while(clerk_arr[*clerk_id]!=0){ // waits   
            }
        } if (business_q_length==0) {
            pthread_mutex_lock(&mutex0);
            econ_q_status = *clerk_id;
            pthread_cond_broadcast(&cond0);
            winner_econ = 0;
            pthread_mutex_unlock(&mutex0);
            while(clerk_arr[*clerk_id]!=0){ // waits
            }
        }
    }
}

// function for customer threads
void *customer_function(void *arg){
	struct customer_info * p_myInfo = (struct customer_info *)arg;
    double queue_enter_time = 0.0;
    int myclerk = 0;
	usleep(p_myInfo->arrival_time*100000);
    printf("A customer arrives: customer ID %d.\n",p_myInfo->user_id);
   
    // for customers in economy class
    if (p_myInfo->class_type==0) {
        pthread_mutex_lock(&mutex0);
        enqueue(p_myInfo->user_id,0);        
        econ_count += 1.0;
        queue_enter_time = getCurrentSimulationTime();
        printf("A customer enters a queue: the queue ID 0, and length of the queue %2d. \n",econ_q_length);
        while (1) {
            pthread_cond_wait(&cond0,&mutex0);
            if (get(0)==p_myInfo->user_id && winner_econ==0) {
                myclerk=econ_q_status;
                clerk_arr[myclerk]=myclerk;
                dequeue(0);
                winner_econ = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutex0);
        usleep(10);
        printf("A clerk starts serving a customer: start time %.2f, the customer ID %d, the clerk ID %d. \n",getCurrentSimulationTime(),p_myInfo->user_id,myclerk);
        usleep(p_myInfo->service_time*100000);
        printf("--->A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %d. \n",getCurrentSimulationTime(),p_myInfo->user_id,myclerk);\
        economy_waiting_time = getCurrentSimulationTime() - queue_enter_time;
        clerk_arr[myclerk] = 0; // flagging idle condition
        pthread_cond_signal(&clerk0); 
        
    // for customers in business class
    } else {
        pthread_mutex_lock(&mutex1);
        enqueue(p_myInfo->user_id,1);
        business_count += 1.0;
        queue_enter_time = getCurrentSimulationTime();
        printf("A customer enters a queue: the queue ID 1, and length of the queue %2d. \n",business_q_length);
        while (1) {
            pthread_cond_wait(&cond1,&mutex1);
            if (get(1)==p_myInfo->user_id && winner_business==0) {
                myclerk = business_q_status;
                clerk_arr[myclerk] = myclerk;
                dequeue(1);
                winner_business = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutex1);
        usleep(10);
        printf("A clerk starts serving a customer: start time %.2f, the customer ID %d, the clerk ID %d. \n",getCurrentSimulationTime(),p_myInfo->user_id,myclerk);
        usleep(p_myInfo->service_time*100000);
        printf("--->A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %d. \n",getCurrentSimulationTime(),p_myInfo->user_id,myclerk);
        business_waiting_time = getCurrentSimulationTime() - queue_enter_time;
        clerk_arr[myclerk] = 0; // flagging idle condition
        pthread_cond_signal(&clerk1);
    }
    overall_waiting_time = economy_waiting_time + business_waiting_time;
	pthread_exit(NULL);
	return NULL;
}


int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("File cannot be opened\n");
        exit(1);
    }

    char line[MAX_SIZE];
    int num_customer = 0;
    if (atoi(fgets(line, sizeof(line),fp)) != -1) {
        num_customer = atoi(line);
    }

    // parsing input file
    struct customer_info customer[num_customer];
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line,"%d:%d,%d,%d",&customer[count].user_id,&customer[count].class_type,&customer[count].arrival_time,&customer[count].service_time);
        count++;
    }
    gettimeofday(&start_time,NULL);
    int num_clerks = 5;
    pthread_t clerk_thread[num_clerks];
    pthread_t customer_thread[num_customer]; 

    // creating clerk thread
    for (int i=0; i<num_clerks; i++){
        int *clerk = malloc(sizeof(int));
        *clerk = i;
        if(pthread_create(&clerk_thread[i-1],NULL,clerk_function,(void *)clerk)!=0){
            printf("error\n");
        }
	}
	
	// creating customer thread
	for (int i=0; i<num_customer; i++){
        if (customer[i].class_type == 0) {
            economic_size++;
        } else {
            business_size++;
        }
        if (pthread_create(&customer_thread[i],NULL,customer_function,&customer[i])!=0) {
            perror("failed to create thread");
        }
	}

    for (int i=0; i<num_customer; i++) {
        if (pthread_join(customer_thread[i], NULL)!=0) {
            perror("failed to join threads");
        }
    }

    // printing concluding statements
    economy_waiting_time /= econ_count;
    business_waiting_time /= business_count;
    printf("All jobs done...\n");
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n",economy_waiting_time);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n",business_waiting_time);
    printf("The average waiting time for all customers in system is: %.2f seconds\n",overall_waiting_time/num_customer);

    fclose(fp);
    return 0;
}
