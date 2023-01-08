#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable_ex.h>

std::mutex m;
gecaib::condition_variable_ex cv;
std::string my_data;
bool ready = false;
bool processed = false;

void worker_thread()
{
    // Wait until main() sends data
    std::unique_lock lk(m);
    auto status = cv.wait_for(lk, chrono::microseconds(1000), []
            { return ready; });

    // after the wait, we own the lock.
    std::cout << "Worker thread is processing data\n";
    my_data += " after processing with status " + std::to_string((int)status);

    // Send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv.notify_one();
}

int main(int, char **)
{
    std::thread worker(worker_thread);
    my_data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();

    // wait for the worker
    {
        std::unique_lock lk(m);
        cv.wait(lk, []
                { return processed; });
    }
    std::cout << "Back in main(), data = " << my_data << '\n';

    worker.join();
}
