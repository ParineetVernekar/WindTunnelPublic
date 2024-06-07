#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <Arduino.h> // Include necessary headers

struct QueueData {
  String type;
  uint64_t timestamp;
  float value;
  String device;
  String run_id;
  int counter;
};

class Queue {

  public:
    Queue(int size);
    ~Queue();
    void enqueue(QueueData item);
    QueueData dequeue();
    void printQueue();
    bool isFull();
    bool isEmpty();

  private:
    int front;
    int rear;
    int max_size;
    int current_size;
    QueueData *queue;

};

#endif // MYQUEUE_H
