// Spinlock allows only a single thread to acquire lock and proceed with the critical section.
// advantages of spin lock
// It does not require a context switch because it is busy waiting, and the thread is not sleeping.
// If the critical section (CS) is smaller, it is helpful.
// disadvantages of spin lock
// Spinlock needs busy waiting.
// When the lock is unavailable, it wastes a CPU cycle and repeatedly checks for it to be accessible.

// Mutex is a locking method, in which a lock can be obtained by a process before entering the CS and released once it is done with the critical section.


// Semaphore is a non-negative integer shared by multiple threads,
//Semaphore works upon signaling mechanism, in this a thread can be signaled by another thread.
// Semaphore uses two atomic operations for process synchronisation: 
// Wait (P)
// Signal (V)

/*
A mutex is a locking mechanism used to synchronize access to a resource. Only one task (can be a thread or process based on OS abstraction) can acquire the mutex. It means there is ownership associated with a mutex, and only the owner can release the lock (mutex). 

Semaphore is a signaling mechanism (“I am done, you can carry on” kind of signal). For example, if you are listening to songs (assume it as one task) on your mobile phone and at the same time, your friend calls you, an interrupt is triggered upon which an interrupt service routine (ISR) signals the call processing task to wakeup. 
*/
