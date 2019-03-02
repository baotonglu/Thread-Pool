# Thread-Pool
This is a simple thread pool implementation that has has some enhanced features such as  "wait_for_task ".

## Feature
You can specifiy the number of threads of the pool and submit your task to the pool. Specifically, You may refer to this [page](https://www.cs.virginia.edu/~cr4bd/4414/S2019/pool.html) to get a full understanding.

## Example of Usage
The repo has a static library named **libpool.a**, you can directly add this lib to your own code. An example of usage is listed below:
   
1. To use the ThreadPool class you create, a user would create a subclass of Task that implements the Run() method that performs an operation they want to add to the queue of operations to do:
``` c
class ComputeSumTask : public Task {
public:
    ComputeSumTask(int *sum_destination, int *array_to_sum, int array_size) {
        this->sum_destination = sum_destination;
        this->array_to_sum = array_to_sum;
        this->array_size = array_size;
    }

    void Run() {
        int sum = 0;
        for (int i = 0; i < this->array_size; ++i) {
            sum += this->array_to_sum[i];
        }
        this->sum_destination = sum;
    }
        
    int *sum_destination,
    int *array_to_sum;
    int array_size; 
};
```
2. Notice that the Task subclass can (and typically would) contain member variables. Then, submit a bunch of instances of this class for each thing they wanted to do in parallel
``` c
int arrayA[ARRAY_SIZE], arrayB[ARRAY_SIZE];
int sum_of_A, sum_of_B;
ThreadPool pool(num_threads);
    
pool.SubmitTask("sum arrayA", new ComputeSumTask(&sum_of_A, arrayA, ARRAY_SIZE));
pool.SubmitTask("sum arrayB", new ComputeSumTask(&sum_of_B, arrayB, ARRAY_SIZE));
```
3. Finally wait for the tasks to complete before stopping the thread pool(**but this step is optional**):
``` c
pool.WaitForTask("sum arrayA");
pool.WaitForTask("sum arrayB");

pool.Stop();
```
## About the Source Code
Some knowledge about **pthread**: before invoking **pthread_cond_signal**, make sure to have the lock held. This ensures that we don't accidentally introduce a race condition into our code.

## Disclaimer
This library is just for study. Full correctness is not guranteed. 
