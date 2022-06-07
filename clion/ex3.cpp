#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <fstream>
#include <unistd.h>
#include <pthread.h>

using namespace std;

typedef struct producerArgs {
    int i;
    int num;
} producerArgs;

typedef struct coEditorArgs {
    int i;
} coEditorArgs;

typedef struct dispatcherArgs {
    int n;
} dispatcherArgs;

class BoundedQueue {
private:
    queue<string> _queue;
    mutex m;
    sem_t full;
    sem_t empty;
public:
    BoundedQueue(int size): _queue() {
        //cout << "constructing bounded queue .. " << endl;
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, size);
        //cout << "done constructing bounded queue !" << endl;
    }

    string pop() {
        //cout << "popping BoundedQueue .. " << endl;
        // SEMAPHORE, MUTEX
        //cout << "deadlock in BoundedQueue pop" << endl;
        sem_wait(&full);
        m.lock();

//        if(_queue.empty()) {
//            m.unlock();
//            sem_post(&empty);
//            return "";
//        }
        string s = _queue.front();
        _queue.pop();

        m.unlock();
        sem_post(&empty);

        //cout << "done popping BoundedQueue !" << endl;
        //cout << "exit deadlock " << endl;
        return s;
        // SEMAPHORE, MUTEX
    }

    void push(const string s) {
        //cout << "pushing BoundedQueue ..." << endl;
        // SEMAPHORE, MUTEX
        //cout << "deadlock in BoundedQueue push" << endl;
        sem_wait(&empty);
        m.lock();

        _queue.push(s);

        m.unlock();
        sem_post(&full);

        //cout << "exit deadlock " << endl;
        // SEMAPHORE, MUTEX
        //cout << "done pushing BoundedQueue !" << endl;
    }
};

class UnboundedQueue {
private:
    queue<string> _queue;
    sem_t m;
    sem_t full;
public:
    UnboundedQueue(): _queue() {
        //cout << "constructing UNBOUNDED queue .. " << endl;
        sem_init(&full,0 ,0);
        sem_init(&m, 0, 1);
        //cout << "done constructing UNBOUNDED queue ! " << endl;
    }

    string pop() {
       // cout << "deadlock in UnboundedQueue pop" << endl;
        //cout << "popping UNBOUNDED Queue .. " << endl;
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
        sem_wait(&full);
        sem_wait(&m);

//        if(_queue.empty()) {
//            m.unlock();
//            return "";
//        }
        string s = _queue.front();
        _queue.pop();

        sem_post(&m);

        //cout << "done popping UNBOUNDED Queue !" << endl;
        //cout << "exit deadlock " << endl;
        return s;
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
    }

    void push(string s) {
        //cout << "pushing UNBOUNDED Queue .. " << endl;
        // SEMAPHORE, MUTEX (no need for "empty", only the "full" and the mutex)
        //cout << "deadlock in UnboundedQueue push" << endl;

        sem_wait(&m);

        _queue.push(s);

        sem_post(&m);
        sem_post(&full);

        //cout << "pushing UNBOUNDED Queue ! " << endl;
       // cout << "exit deadlock " << endl;

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



void producer(producerArgs * producerArgs1) {
    int i = producerArgs1->i;
    int num = producerArgs1->num;

    //cout << "Producing in Progress 1 : " << i << endl;

    const string subjects[] = {"SPORTS", "NEWS", "WEATHER"};
    int created[] = {0, 0, 0};

    for(int j=0; j<num ; j++) {
        //cout << "Again !" << endl;
        //cout << "in loop " << j << endl;
        string article = "producer ";

        // add producer id
        article += to_string(i);
        article += " ";

        //cout << "randomizing.. " << endl;
        // randomize subject
        int k = rand() % 3;
        article += subjects[k];
        article += " ";

        // add the number of article made for this type of subject
        article += to_string(created[k]);
        article += "\n";

        //cout << "pushing.. " << endl;
        created[k] ++;
        producers.at(i)->push(article);
        //bq[i].push(article);
    }

    //cout << "done looping in producer number " << i << "! pushing -1" << endl;
    // notify dispatcher i am done
    producers.at(i)->push("-1");
    delete producerArgs1;
    //bq[i].push("-1");
}

void dispatcher(dispatcherArgs * dispatcherArgs1) {
    //cout << "DISPATCHER STARTED" << endl;
    int n = dispatcherArgs1->n;
    int done = 0;

    // ROUND ROBIN

    vector<bool> manage(n, true);


    //bool finished = true;

    while(done != n) {
        //cout << "Again and AGAIN!" << endl;
        //cout << "looping in dispatcher .. " << endl;
        //finished = true;
        for(int i=0; i<n; i++) {
            //cout << "for loop number " << i << " in dispatcher" << endl;
            if(manage[i]) {
                //finished = false;

                // WAIT FOR bq[i] IF QUEUE IS EMPTY - producer

                string s = producers.at(i)->pop();
                //string s = bq[i].pop();
                if (s == "-1") {
                    // empty stack

//                    int j = i;
//                    s = producers.at(i)->pop();
//                    while(s.length() != 0) {
//                        cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"<< j++ << endl;
//                        if(s.find("SPORTS") != string::npos) co_editors[0].push(s);
//                        if(s.find("NEWS") != string::npos) co_editors[1].push(s);
//                        if(s.find("WEATHER") != string::npos) co_editors[2].push(s);
//                        s = producers.at(i)->pop();
//                    }
                    done++;
                    manage[i] = false;
                    continue;
                }
                if(s.length() == 0) continue;

                // i can add a printf and sleep for 0.1 sec to see the outcome better
                if(s.find("SPORTS") != string::npos) co_editors[0].push(s);
                if(s.find("NEWS") != string::npos) co_editors[1].push(s);
                if(s.find("WEATHER") != string::npos) co_editors[2].push(s);
            }
        }
        //if(finished) {

        //}
    }
    // notify the co-editors that the job is done
    co_editors[0].push("-1");
    co_editors[1].push("-1");
    co_editors[2].push("-1");

    //cout << "DISPATCHER DONE" << endl;
    delete dispatcherArgs1;

    cout << endl << "FINISHED DISPTACHER" << endl <<endl;
}

void co_editor(coEditorArgs * coEditorArgs1) {
    int i = coEditorArgs1->i;
    //cout << "starting co editor number "<< i << endl;
    // unbounded
    while(1) {

        //cout << "start looping in co editor... " << endl;
        string s = co_editors[i].pop();
        if (s == "-1") {
            cout << "Again and AGAIN!" << endl;
            // notify screen manager that the job is done
            common_queue->push("-1");
            break;
        }

        if(s.length() == 0) continue;

        // i can add a printf and sleep for 0.1 sec to see the outcome better
        //usleep(0.1 * 1000000);

        // co-editors common bounded queue
        common_queue->push(s);
    }
    //cout << "done CO EDITOR number " << i << endl;
    delete coEditorArgs1;
}

void screen_manager() {
    //cout << "starting screen manager ..." << endl;
    int finished_num = 0;
    while(1) {
        //cout << "start looping in screen manager ... " << endl;
        string s = common_queue->pop();
        if(s == "-1") {
            finished_num++;
            // check if all co-editors are done
            if(finished_num == 3) break;
            continue;
        }
        if(s.length() == 0) continue;

        // print the string
        cout << s << endl;
    }
    cout << "finished screen manager !" << endl;
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
            producerArgs* producerArgs1 = new producerArgs ;
            producerArgs1->i = count;
            producerArgs1->num = articles_number;
            thread (producer,producerArgs1).detach();

            // continue to the next producer
            count++;
        }
        if(line.find("Co-Editor", 0) == 0) {
            string s_editor_size = line.substr(23);
            int editor_queue_size = stoi(s_editor_size);
            // initialize co editors common queue
            common_queue = new BoundedQueue(editor_queue_size);

            // dispatcher
            dispatcherArgs * dispatcherArgs1 = new dispatcherArgs;
            dispatcherArgs1->n = count;
            thread (dispatcher,dispatcherArgs1).detach();

            // sports
            coEditorArgs* coEditorArgs1 = new coEditorArgs ;
            coEditorArgs1->i = 0;
            thread (co_editor, coEditorArgs1).detach();

            // news
            coEditorArgs* coEditorArgs2 = new coEditorArgs ;
            coEditorArgs1->i = 1;
            thread (co_editor, coEditorArgs2).detach();

            // weather
            coEditorArgs* coEditorArgs3 = new coEditorArgs ;
            coEditorArgs1->i = 2;
            thread (co_editor, coEditorArgs3).detach();
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