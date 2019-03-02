#include "pool.h"


Task::Task() {
}

Task::~Task() {
}

ThreadPool::ThreadPool(int num_threads) {
	/* Initialize*/
	int err;
	thread_num = num_threads;
	task_num = 0;
	shut_down = false;
	/*
	queue_lock = PTHREAD_MUTEX_INITIALIZER;
	map_lock = PTHREAD_MUTEX_INITIALIZER;
	notify = PTHREAD_COND_INITIALIZER;
	*/
	err = pthread_mutex_init(&queue_lock, NULL);
	assert(err == 0);
	err = pthread_mutex_init(&map_lock, NULL);
	assert(err == 0);
	err = pthread_cond_init(&notify, NULL);
	assert(err == 0);

	thread_array = (pthread_t *)malloc(sizeof(pthread_t)*num_threads);
	for (int i = 0; i < thread_num; ++i)
	{
		/* create threads */
		pthread_create(thread_array+i, NULL, ThreadPool::wrapper, this);
	}
}

/* Since pthread_create only accept static funciton, so we need this wrraper function*/
void * ThreadPool::wrapper(void *object){
	ThreadPool *this_object = (ThreadPool *)object;
	this_object->ThreadLife(NULL);
	return NULL;
}

void * ThreadPool::ThreadLife(void *){
	queue_node *my_node = NULL;
	std::map<std::string, pthread_cond_t *>::iterator iter;
	do{
		 pthread_mutex_lock(&queue_lock);
		 
		 /* After this task is worken up, it has to check if the task_num is not zero, else
		 	it still needs to sleep!*/
		 while(task_num == 0 && !shut_down)
		 {
		 	/* sleep */
		 	pthread_cond_wait(&notify, &queue_lock);
		 } 

		if (task_num == 0 && shut_down)
		{
			pthread_mutex_unlock(&queue_lock);
			break;
		}

		assert(task_num > 0);
		my_node = pool_queue.front();
		pool_queue.pop_front();
		task_num--;
		assert(my_node != NULL);
		//printf("I have gotten the task\n");
		 /* Add the task into the map, need to first check if there had been another thread is waiting*/
		pthread_mutex_lock(&map_lock);
		if (pool_map.find(my_node->name) == pool_map.end())
		{
			pool_map.insert(std::pair<std::string, pthread_cond_t *>(my_node->name, NULL));
		}
		pthread_mutex_unlock(&map_lock);
		//printf("add myself to the map\n");
		 /* Run the task*/
		pthread_mutex_unlock(&queue_lock);
		//printf("begin running task\n");
		my_node->my_task->Run();
		//printf("finished running task\n");

		/* Check if a thread is waiting for my finishment*/
		pthread_mutex_lock(&map_lock);
		iter = pool_map.find(my_node->name);
		if (iter->second != NULL)
		{
			pthread_cond_signal(iter->second);	
		}
		pool_map.erase(my_node->name);	
		pthread_mutex_unlock(&map_lock);

		/* Finish*/
		delete my_node->my_task;
		delete my_node;
	}while(1);

	return NULL;
}

void ThreadPool::SubmitTask(const std::string &name, Task* task) {
	queue_node *my_node = new queue_node;
	my_node->name = name;
	my_node->my_task = task;

	pthread_mutex_lock(&queue_lock);
	pool_queue.push_back(my_node);
	task_num++;
	/* After I add the task to the pool, I need to wake up the task*/
	pthread_cond_signal(&notify);
	pthread_mutex_unlock(&queue_lock);
}

void ThreadPool::WaitForTask(const std::string &name) {
	std::list<queue_node *>::iterator iter;
	std::map<std::string, pthread_cond_t *>::iterator map_iter;
	bool find_flag = false;
	/* Check the pool_queue first*/
	pthread_mutex_lock(&queue_lock);
	for (iter = pool_queue.begin(); iter != pool_queue.end(); iter++)
	{
		if ((*iter)->name == name)
		{
			find_flag = true;
			break;
		}
	}

	if (find_flag)
	{
		/* Add myself to the waiting map and wait for the task finishment*/
		pthread_mutex_lock(&map_lock);
		pthread_cond_t wait_task = PTHREAD_COND_INITIALIZER;
		pool_map.insert(std::pair<std::string, pthread_cond_t *>(name, &wait_task));
		pthread_mutex_unlock(&queue_lock);
		pthread_cond_wait(&wait_task, &map_lock);
		pthread_mutex_unlock(&map_lock);
		return;
	}else{
		pthread_mutex_unlock(&queue_lock);
	}

	/* Check if it's processing*/
	pthread_mutex_lock(&map_lock);
	if ((map_iter = pool_map.find(name)) != pool_map.end())
	{
		pthread_cond_t wait_task = PTHREAD_COND_INITIALIZER;
		map_iter->second = &wait_task;
		pthread_cond_wait(&wait_task, &map_lock);
	}
	pthread_mutex_unlock(&map_lock);
}

void ThreadPool::Stop() {
	shut_down = true;
	pthread_cond_broadcast(&notify);
	for (int i = 0; i < thread_num; ++i)
	{
		/* wait for thread_join */
		pthread_join(thread_array[i], NULL);
	}
	assert(task_num == 0);
	free(thread_array);
	/* Destroy the conditional variable and mutex*/
	pthread_mutex_destroy(&queue_lock);
	pthread_mutex_destroy(&map_lock);
	pthread_cond_destroy(&notify);
}
