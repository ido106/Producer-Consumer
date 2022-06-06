#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <random>
#include <fstream>
#include <unistd.h>

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

// producers
//BoundedQueue bq[N];
vector<BoundedQueue*> producers;

// CO-EDITORS: sports-1 news-2 weather-3
UnboundedQueue co_editors[3];
// common queue for all co-editors
BoundedQueue* common_queue;



void producer(int i, int num) {
    cout << "Producing in " << i << endl;
    // todo i can add a printf and sleep for 0.1 sec to see the outcome better

    const string subjects[] = {"SPORTS", "NEWS", "WEATHER"};
    int created[] = {0, 0, 0};

    for(int j=0; j<num ; j++) {
        string article = "producer ";

        // add producer id
        article += i;
        article += " ";

        // randomize subject
        int k = rand() % 3;
        article += subjects[k];
        article += " ";

        // add the number of article made for this type of subject
        article += created[k];
        article += "\n";

        created[k] ++;
        producers.at(i)->push(article);
        //bq[i].push(article);
    }

    // notify dispatcher i am done
    producers.at(i)->push("-1");
    //bq[i].push("-1");
}

void dispatcher(int n) {
    // ROUND ROBIN

    vector<bool> manage(n, true);

    bool finished = true;

    while(1) {
        finished = true;
        for(int i=0; i<n; i++) {
            if(manage[i]) {
                finished = false;

                // WAIT FOR bq[i] IF QUEUE IS EMPTY - producer

                string s = producers.at(i)->pop();
                //string s = bq[i].pop();
                if (std::equal(s.begin(), s.end(),"-1")) {
                    manage[i] = false;
                    continue;
                }

                // todo i can add a printf and sleep for 0.1 sec to see the outcome better
                if(std::equal(s.begin(), s.end(), "s")) co_editors[0].push(s);
                if(std::equal(s.begin(), s.end(), "n")) co_editors[1].push(s);
                if(std::equal(s.begin(), s.end(),"w")) co_editors[2].push(s);
            }
        }
        if(finished) {
            // notify the co-editors that the job is done
            co_editors[0].push("-1");
            co_editors[1].push("-1");
            co_editors[2].push("-1");
            break;
        }
    }
}

void co_editor(int i) {
    // unbounded
    while(1) {
        string s = co_editors[i].pop();
        if (std::equal(s.begin(), s.end(),"-1")) {
            // notify screen manager that the job is done
            common_queue->push("-1");
            break;
        }
        // todo i can add a printf and sleep for 0.1 sec to see the outcome better
        usleep(0.1 * 1000000);
        // co-editors common bounded queue
        common_queue->push(s);
    }
}

void screen_manager() {
    int finished_num = 0;
    while(1) {
        string s = common_queue->pop();
        if(std::equal(s.begin(), s.end(),"-1")) {
            finished_num++;
            // check if all co-editors are done
            if(finished_num == 3) break;
        }

        // todo print to screen the articles
        printf("have to print something....");
    }
}

void free_all() {
    for(auto q : producers) {
        delete q;
    }
    delete common_queue;
}

void produce(string path) {
    ifstream stream(path);
    string line;
    int count = 0;
    while(getline(stream, line)) {
        // if the line is blank
        if(line.size() ==0) {
            continue;
        }

        // find PRODUCER
        if(line.find("PRODUCER", 0) == 0) {
            getline(stream, line);
            // get the number of articles
            int articles_number = stoi(line);

            // get the queue size
            getline(stream, line);
            string s_size = line.substr(13);
            int queue_size = stoi(s_size);

            // initialize the producer thread
            producers.push_back(new BoundedQueue(queue_size));
            thread(producer, count, articles_number);

            // continue to the next producer
            count++;
            continue;
        }

        if(line.find("Co-Editor", 0) == 0) {
            string s_editor_size = line.substr(24);
            int editor_queue_size = stoi(s_editor_size);
            // initialize co editors common queue
            common_queue = new BoundedQueue(editor_queue_size);

            // sports
            thread sports_co_editor(co_editor, 1);
            // news
            thread news_co_editor(co_editor, 2);
            // weather
            thread weather_co_editor(co_editor, 3);

            thread disp(dispatcher,count);
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc<2) {
        printf("Not enough arguments \n");
        return 0;
    }

    string config_path = argv[1];
    produce(config_path);

    //thread disp(dispatcher);

    thread manager(screen_manager);
    manager.join();

    free_all();
    return 0;
}