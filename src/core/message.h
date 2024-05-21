#pragma once

#include <queue>
#include <mutex>

enum MessageType{
    MSG_INVALID
};

class Message{
public:
    //Type of message
    MessageType type;
    //Pointer to the data held by the message. The message system will not interact with this in any way
    void* data;

    Message(MessageType a_type, void* a_data = nullptr)
        :type(a_type), data(a_data){}
};

class MessageQueue{
public:
    /// @brief Adds a message to the queue
    /// @param a_message 
    static void push(Message a_message){
        //Lock the queue with the mutex for thread safety
        std::lock_guard<std::mutex> lock(mutex);

        //Push the message into the queue
        queue.push(std::move(a_message));
    }

    static Message pop(){
        //Lock the queue with the mutex for thread safety
        std::lock_guard<std::mutex> lock(mutex);
        
        //Ensure the thread isn't empty
        if(queue.empty())
            return Message(MessageType::MSG_INVALID);
        
        //Consume the front message from the queue
        Message msg = std::move(queue.front());
        queue.pop();
        //Return the message to the caller
        return msg;
    }

private:
    static std::queue<Message> queue;
    static std::mutex mutex;
};