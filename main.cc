/**
        C++ Producer Consumer using C++11 thread facilities
        To compile: g++ -std=c++11 <program name> -pthread -lpthread -o pc
*/
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <thread>

#include "helper.h"

using namespace std;

// print function for "thread safe" printing using a stringstream
void print(ostream& s) { cout << s.rdbuf(); cout.flush(); s.clear(); }

//
//      Constants
//

const int consumer_max_wait_time = 15000;     // in miliseconds - max time that a consumer can wait for a product to be produced.

int num_producers;
int num_consumers;
int max_jobs;                               // When producers has produced this quantity they will stop to produce
int queue_size;                             // Maximum number of products that can be stored

//
//      Variables
//
atomic<int> num_producers_working(0);       // When there's no producer working the consumers will stop, and the program will stop.
stack<pair<int, int>> products;             // The products stack, here we will store our products. Each product is a pair product_id + duration
mutex xmutex;                               // Our mutex, without this mutex our program will cry

condition_variable is_not_full;             // to indicate that our stack is not full between the thread operations
condition_variable is_not_empty;            // to indicate that our stack is not empty between the thread operations

//
//      Functions
//

//      Produce function, producer_id will produce a product
void produce(int producer_id)
{
    unique_lock<mutex> lock(xmutex);
    pair<int,int> product;

    //is_not_full.wait(lock, [] { return products.size() != queue_size; });
    product.first = rand() % queue_size;
    product.second = rand() % 10 + 1; // 1 to 10 seconds duration
    products.push(product);

    //Producer(1): Job id 2 duration 10

    stringstream s = stringstream() << "Producer(" << producer_id+1 << "): Job id " << product.first << " duration " << product.second << "\n";
    print(s);


    is_not_empty.notify_all();
}

//      Consume function, consumer_id will consume a product
void consume(int consumer_id)
{
    unique_lock<mutex> lock(xmutex);
    pair<int, int> product;

    if(is_not_empty.wait_for(lock, chrono::milliseconds(consumer_max_wait_time),
                             [] { return products.size() > 0; }))
    {
        product = products.top();
        products.pop();

        //Consumer(2): Job id 3 executing sleep duration 4
        stringstream s = stringstream() << "Consumer(" << consumer_id+1 << "): Job id " << product.first << " executing sleep duration " << product.second << "\n";
        print(s);
        is_not_full.notify_all();

        this_thread::sleep_for(chrono::milliseconds(product.second * 1000));
    }

}

//      Producer function, this is the body of a producer thread
void producer(int id)
{
    ++num_producers_working;
    for(int i = 0; i < max_jobs; ++i)
    {

        int rand_delay = rand() % 5 + 1; // Rand delay 1 to 5 s

        produce(id);
        this_thread::sleep_for(chrono::milliseconds(rand_delay * 1000));
    }


    //Producer(2): No more jobs to generate.
    stringstream s = stringstream() << "Producer(" << id+1 << ") no more jobs to generate.\n";
    print(s);
    --num_producers_working;
}

//      Consumer function, this is the body of a consumer thread
void consumer(int id)
{
    // Wait until there is any producer working
    while(num_producers_working == 0) this_thread::yield();

    while(num_producers_working != 0 || products.size() > 0)
    {
        consume(id);
    }

    //Consumer(3): No more jobs left.
    stringstream s = stringstream() << "Consumer(" << id+1 << "): No more jobs left.\n";
    print(s);
}

//
//      Main
//
int main (int argc, char **argv)
{

    //Seed the random sleep times
    srand(time(0));

    if(argc != 5)
    {
        cout << "Wrong argument list" << endl;
        exit(-1);
    }

    queue_size = check_arg(argv[1]);
    if(queue_size < 0)
    {
        cout << "Invalid queue size " << argv[1] << endl;
        exit(-1);
    }

    max_jobs = check_arg(argv[2]);
    if(max_jobs < 0)
    {
        cout << "Invalid job count " << argv[2] << endl;
        exit(-1);
    }

    num_producers = check_arg(argv[3]);
    if(num_producers < 0)
    {
        cout << "Invalid producers count " << argv[3] << endl;
        exit(-1);
    }

    num_consumers = check_arg(argv[4]);
    if(num_consumers < 0)
    {
        cout << "Invalid consumers count " << argv[4] << endl;
        exit(-1);
    }

    vector<thread> producers_and_consumers;

    // Create producers
    for(int i = 0; i < num_producers; ++i)
        producers_and_consumers.push_back(thread(producer, i));

    // Create consumers
    for(int i = 0; i < num_consumers; ++i)
        producers_and_consumers.push_back(thread(consumer, i));

    // Wait for consumers and producers to finish
    for(auto& t : producers_and_consumers)
        t.join();

  return 0;
}

