#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(myMutex); 
    _condition.wait(lock, [this] {return !_queue.empty(); });
    T msg = std::move(_queue.front());

    _queue.pop_front();

    return std::move(msg);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> _lock(myMutex);
    _queue.emplace_back(msg);
    _condition.notify_one();
}



/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(!(TrafficLight::getCurrentPhase()==green)){
        TrafficLightPhaseMsgQueue.receive();
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    //To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
//Declare cycle min/max value  
float maxCycle = 6; 
float minCycle = 4;
//Declare time tracking
auto lastUpdate = std::chrono::system_clock::now();
auto lastChange = std::chrono::system_clock::now();

//Declare

//Suggested use of std::mt19937 - something to look into
float cycleDuration = maxCycle + static_cast<float> (rand())/(static_cast<float> (RAND_MAX/(maxCycle-minCycle)));;
//Infinite loop
    while(true){
std::this_thread::sleep_for(std::chrono::milliseconds(1));
long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(lastUpdate - lastChange).count();

        //If lights green then make it red
        if(_currentPhase == TrafficLightPhase::green && (timeSinceLastUpdate >= cycleDuration)){
            _currentPhase = TrafficLightPhase::red;
            //send w move semantics
              TrafficLightPhaseMsgQueue.send(std::move(TrafficLightPhase::red));
            lastChange = std::chrono::system_clock::now(); //Reset clock
        //If lights red then make it greeen 
        } else if (_currentPhase == TrafficLightPhase::red && (timeSinceLastUpdate >= cycleDuration)){
            _currentPhase = TrafficLightPhase::green;
            //send w move semantics
            TrafficLightPhaseMsgQueue.send(std::move(TrafficLightPhase::green));
            lastChange = std::chrono::system_clock::now(); //Reset clock
        }
        lastUpdate = std::chrono::system_clock::now();
    }
}
