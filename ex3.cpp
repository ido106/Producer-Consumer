#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <semaphore.h>

using namespace std;

class BoundedQueue {
private:
    queue<string> _queue;
    mutex m;
    sem_t full;
    sem_t empty;
public:
    BoundedQueue(int size): _queue() {
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, size);
    }

    string pop() {
        // SEMAPHORE, MUTEX
        sem_wait(&full);
        m.lock();

        if(_queue.empty()) {
            m.unlock();
            sem_post(&empty);
            return nullptr;
        }
        string s = _queue.front();
        _queue.pop();

        m.unlock();
        sem_post(&empty);

        return s;
        // SEMAPHORE, MUTEX
    }

    void push(string s) {
        // SEMAPHORE, MUTEX
        sem_wait(&empty);
        m.lock();

        _queue.push(s);

        m.unlock();
        sem_post(&full);
        // SEMAPHORE, MUTEX
    }
};

class UnboundedQueue {
private:
    queue<string> _queue;
    mutex m;
    sem_t full;
public:
    UnboundedQueue(): _queue() {
        sem_init(&full,0 ,0);
    }

    string pop() {
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
        sem_wait(&full);
        m.lock();

        if(_queue.empty()) {
            m.unlock();
            sem_post(&full);
            return nullptr;
        }
        string s = _queue.front();
        _queue.pop();

        m.unlock();

        return s;
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
    }

    void push(string s) {
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)

        m.lock();

        _queue.push(s);

        m.unlock();
        sem_post(&full);

        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
    }
};

// queues number
const int N = 10;

// producers
BoundedQueue bq[N];
// CO-EDITORS: sports-1 news-2 weather-3
UnboundedQueue ubq[3];
// common queue for all co-editors
BoundedQueue common_queue;

void producer(int i) {
    // todo i can add a printf and sleep for 0.1 sec to see the outcome better
    // bq[i]
}

void dispatcher() {
    // ROUND ROBIN

    bool manage[N] = {true};
    bool finished = true;

    while(1) {
        finished = true;
        for(int i=0; i<N; i++) {
            if(manage[i]) {
                finished = false;

                // WAIT FOR bq[i] IF QUEUE IS EMPTY - producer

                string s = bq[i].pop();
                if (std::equal(s.begin(), s.end(),"-1")) {
                    manage[i] = false;
                    continue;
                }

                // todo i can add a printf and sleep for 0.1 sec to see the outcome better
                if(std::equal(s.begin(), s.end(), "s")) ubq[0].push(s);
                if(std::equal(s.begin(), s.end(), "n")) ubq[1].push(s);
                if(std::equal(s.begin(), s.end(),"w")) ubq[2].push(s);
            }
        }
        if(finished) {
            // notify the co-editors that the job is done
            ubq[0].push("-1");
            ubq[1].push("-1");
            ubq[2].push("-1");
            break;
        }
    }
}

void co_editor(int i) {
    // unbounded
    while(1) {
        string s = ubq[i].pop();
        if (std::equal(s.begin(), s.end(),"-1")) {
            // notify screen manager that the job is done
            common_queue.push("-1");
            break;
        }
        // todo i can add a printf and sleep for 0.1 sec to see the outcome better
        // co-editors common bounded queue
        common_queue.push(s);
    }
}

void screen_manager() {
    int finished_num = 0;
    while(1) {
        string s = common_queue.pop();
        if(std::equal(s.begin(), s.end(),"-1")) {
            finished_num++;
            // check if all co-editors are done
            if(finished_num == 3) break;
        }

        // todo print to screen the articles
        printf("have to print something....");
    }
}

int main(int argc, char *argv[]) {
    for(int i=0; i<N; i++) {
        thread prod(producer, i);
    }

    thread disp(dispatcher);

    // sports
    thread sports_co_editor(co_editor, 1);
    // news
    thread news_co_editor(co_editor, 2);
    // weather
    thread weather_co_editor(co_editor, 3);

    thread manager(screen_manager);

    // todo wait for producers ?
    // necessary ?
    //disp.join();
    //sports_co_editor.join();
    //news_co_editor.join();
    //weather_co_editor.join();

    // this is necessary
    manager.join();

    return 0;
}