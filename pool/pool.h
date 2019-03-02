#ifndef POOL_H_
#include <string>
#include <pthread.h>
#include <assert.h>
#include <list>
#include <map>


class Task {
public:
    Task();
    virtual ~Task();

    virtual void Run() = 0;  // implemented by subclass
};

struct queue_node{
    std::string name; 
    Task *my_task;
};

class ThreadPool {
public:
    ThreadPool(int num_threads);

    // Submit a task with a particular name.
    void SubmitTask(const std::string &name, Task *task);
 
    // Wait for a task by name, if it hasn't been waited for yet. Only returns after the task is completed.
    void WaitForTask(const std::string &name);

    // Stop all threads. All tasks must have been waited for before calling this.
    // You may assume that SubmitTask() is not caled after this is called.
    void Stop();
private:
    static void *wrapper(void *);
    void *ThreadLife(void *);
    /* Declare*/
    int thread_num;
    int task_num;
    pthread_mutex_t queue_lock;
    pthread_mutex_t map_lock;
    pthread_cond_t notify;
    pthread_t *thread_array;
    bool shut_down;
    /* Task queue*/
    std::list<queue_node *> pool_queue;
    /* Processing map*/
    std::map<std::string, pthread_cond_t *> pool_map;
};
#endif
