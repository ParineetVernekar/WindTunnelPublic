#include <Arduino.h>
#include "queue.h"

Queue::Queue(int size) {
  queue = new QueueData[size];  // Dynamically allocate memory for the queue
  front = 0;
  rear = 0;
  max_size = size;
  current_size = 0;
}

Queue::~Queue() {
  delete[] queue;
}

bool Queue::isFull(){
  return (current_size == max_size);
}

bool Queue::isEmpty(){
  return (current_size == 0);
}

void Queue::enqueue(QueueData item) {
  if (current_size == max_size) {
    // Queue is full, handle accordingly (e.g., print error or discard item)
  } else {
    queue[rear] = item;
    rear = (rear + 1) % max_size;
    current_size++;
  }
}

QueueData Queue::dequeue() {
  if (current_size == 0) {
    // Queue is empty, handle accordingly (e.g., return a default value or print error)
    return QueueData();  // Return default QueueData object
  } else {
    QueueData data = queue[front];
    front = (front + 1) % max_size;
    current_size--;
    return data;
  }
}

void Queue::printQueue() {
  Serial.println("Current Queue:");

  int index = front;

  for (int i = 0; i < current_size; ++i) {
    Serial.print("Index ");
    Serial.print(index);
    Serial.print(", Timestamp = ");
    Serial.print(queue[index].timestamp);
    Serial.print(", Sensor Name = ");
    Serial.println(queue[index].device);

    index = (index + 1) % max_size;  // Move to the next index in a circular manner
  }
}
