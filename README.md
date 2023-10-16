# HEIG_PCO_Labo02

**Authors**: Aubry Mangold, Timothée Van Hove

**Date**: 17.10.2023



## Introduction

We are interested in developing a program capable of cracking an md5 hash to recover a password. An application has been provided to us, but suffers from some performance shortcomings, as it is single-threaded. So the goal of this lab is to enhance the performance of the application by implementing a multi-threaded process.



## How to speed-up the hacking process?

It is evident that the hacking program's primary function is to evaluate every possible combinations. This, being a computationally heavy task, we can leverage concurrency to speed-up the process by allowing simultaneous evaluation of distinct combinations.

**Partitioning Strategy:** The total workload, which is the entire space of combinations, is divided into distinct segments. This approach is chosen over dynamic allocation because it reduces overhead. Each thread is assigned a specific range of combinations to evaluate. This range is determined by a unique identifier, which ensures no two threads work on the same combination, eliminating redundancy and the potential for race conditions.

**Hash Computation:** The nature of hash computation is inherently independent for each combination. This makes it particularly suitable for multi-threading, as no thread needs to wait for data or results from another. Moreover, with the distinct segments, each thread can compute its hash values independently without any synchronization overhead.

**Communication and Feedback:** While the tasks were separate, it is crucial to maintain a communication channel to the main thread. As soon as a thread finds the correct combination, it needed to relay this information to prevent other threads from continuing redundant computations. This immediate feedback mechanism not only accelerates the overall process but also conserves computational resources.

**Handling Skewness:** Although the division was designed to be uniform, computational skewness (if some threads finish earlier than others), was a consideration. The distinct segments method alleviates this by ensuring that each thread has a nearly equal amount of work. While there might be minor variations due to system specifics, the overall efficiency is maintained.



## Workflow

Here is a simplified representation of the workflow:

![](figures/process_diagram.png)



The execution begins with the `MainWindow` initiating the `prepareHacking()` function, which sets the initial parameters and readies the environment for the upcoming multithreading operations.

The control is then transferred to the `ThreadManager` via the `startHacking()` function. Here, the primary responsibility is to manage and coordinate the multi-threaded operations. Within `ThreadManager`, the `setUpWork()` method is responsible for allocating distinct segments of the task to individual threads, ensuring no overlap or redundancy. This distribution utilizes the potential of concurrency to enhance efficiency.

Once the task distribution is complete, the `startWork()` method triggers each thread to execute its assigned operations. The `run()` function is executed, which sequentially goes through the steps of hacking. It starts by generating combinations using `idToCombination()`, followed by the computation of the hash with `computeHash()`.

During this process, if a thread identifies a matching hash, it communicates to the main thread, which subsequently executes the `setFoundPassword()` function, logging the identified password. In the event a match isn't found within an iteration, the `progressCallback()` method is utilized to relay the status to the `ThreadManager`.

Post-completion of tasks by the threads or upon successful identification of the hash, the `ThreadManager` invokes the `cancelWork()` function for all active threads, instructing them to terminate their operations. This is immediately followed by the `joinThreads()` function, ensuring that all the threads conclude and system resources are released.

The process is finalized with the `endHacking()` function, which handles any necessary post-operation tasks and concludes the overall workflow. The design emphasis is on efficient distribution of tasks across threads, real-time monitoring, and structured resource management.



## From Work Stealing to Partitioning

In our preliminary design, we employed the "work stealing" strategy, where each thread would fetch a new chunk of work from a shared queue upon completing its segment. To facilitate this, the combination space was fragmented into smaller units, all stored within a concurrency-safe queue accessible by all threads.

However, on close analysis, we concluded that the efficiency gains from this method were minimal, amounting to about 10-20 ms speed-up for a 4-character password. Given this performance enhancement and the complexities introduced by ensuring thread-safe access to the shared queue using mutexes, it became evident that the benefits did not justify the added complexity.

Consequently, we decided to implement the more straightforward partitioning strategy.



## Performance tests

TODO



## Tested scenarios

All the scenarios below have been checked:

- [x] The application must find the original password and notify the user
- [x] If the password is not found the application must notify the user
- [x] The multi-threaded application must be faster than the original single-threaded one
- [x] The application must not crash if the password length, or the hash value are erroneous
- [x] The status bar must report the progress

