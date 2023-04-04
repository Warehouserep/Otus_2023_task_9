#include "async.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>


namespace async {

handle_t connect(std::size_t bulk) {

    return new Context(bulk);
}

void receive(handle_t handle, const char *data, std::size_t size) {
    Context* context = (Context*)handle;
    context->add_commands(data,size); 
    context->process_commands();

}

void disconnect(handle_t handle) {
    Context* context = (Context*)handle;
    context->disconnect();
    
    delete context;
}

}
