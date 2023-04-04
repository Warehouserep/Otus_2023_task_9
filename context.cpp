#include "context.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>


namespace async{
    

Context::Context(std::size_t bulk_size){
    last_command = "";
    bracket_counter = 0;
    this->bulk_size = bulk_size;
    finish_flag = 0;
    file_thread_1 = std::thread(&Context::write_commands, this, "_File1_");
    file_thread_2 = std::thread(&Context::write_commands, this, "_File2_");
    log_thread = std::thread(&Context::log_commands, this);
}
Context::~Context(){
    
}

void Context::log_commands(){
    while(!finish_flag){
        std::unique_lock<std::mutex> locker(query_lock);
        command_check.wait(locker);
        if(command_blocks.size() > 0){
            std::cout << command_blocks.front() << std::endl;
            command_blocks_logged.push(command_blocks.front());
            command_blocks.pop();
            queue_logged_check.notify_all();

        }
    }
    while(!command_blocks.empty()){
        std::cout << command_blocks.front() << std::endl;
        command_blocks_logged.push(command_blocks.front());
        command_blocks.pop();
    }
}

void Context::add_commands(const char *data, std::size_t size)
{
    last_command += std::string(data);
    size_t offset = 0;
    size_t offset_new = 0;
    while ((offset = last_command.find_first_not_of("\n", offset_new)) != std::string::npos)
    {
        offset_new = last_command.find_first_of("\n", offset);
        if(offset_new == std::string::npos) {
            offset_new = offset;
            break;
        }
        std::string  separate_command = last_command.substr(offset,offset_new - offset);
        commands.push_back(separate_command);
    }
    last_command = last_command.substr(offset_new,last_command.length());
}

void Context::process_commands()
{
    size_t i = 0;
    for (i = 0; i < commands.size(); i++){
        if(commands[i] == "{"){
            if(bracket_counter == 0){
                group_command(current_block);
                current_block.clear();
            }
            bracket_counter++;
        }
        else if(commands[i] == "}"){
            bracket_counter--;
            if(bracket_counter == 0){
                group_command(current_block);
                current_block.clear();
            }
            if(bracket_counter < 0) 
                bracket_counter = 0; 
        }
        else {
            if(current_block.size() == 0){
                first_command_time = time(nullptr);
            }
            current_block.push_back(commands[i]);
            if(bracket_counter == 0 && current_block.size() == bulk_size){
                group_command(current_block);
                current_block.clear();
            }
        }
    }
    while(i){
        commands.erase(commands.begin());
        i--;
    }
    command_check.notify_one();
}

void Context::group_command(std::vector<std::string> v)
{
    if (v.size() == 0) return;
    std::string command_group = "bulk: " + v[0];
    for (size_t i = 1; i < v.size(); i++){
        command_group += " " + v[i];
    }
    command_blocks.push(command_group);
    v.clear();
}

void Context::disconnect()
{
    size_t offset, offset_new;
    offset = last_command.find_first_of("\n");
    offset_new = last_command.find_last_of("\n");
    if(offset != std::string::npos && offset_new != std::string::npos)
    commands.push_back(last_command.substr(offset, offset_new - offset));
    process_commands();
    if(bracket_counter == 0){
        group_command(current_block);
    }
    finish_flag = 1;

    log_thread.join();

    queue_logged_check.notify_all();

    file_thread_1.join();
    file_thread_2.join();

}

void Context::write_commands(std::string thread_id){
    while (!finish_flag)
    {   
        std::unique_lock<std::mutex> locker(query_lock);
        if(command_blocks_logged.size()>0){
            std::string command;
            command_blocks_mutex.lock();
            command = command_blocks_logged.front();
            command_blocks_logged.pop();
            command_blocks_mutex.unlock();
            std::string filename = "bulk" + std::to_string(first_command_time) + thread_id + std::to_string(rand()) + ".log";
            std::ofstream file_out(filename);
            file_out << command;
            file_out.close();
            if(command_blocks_logged.size()>0){
                queue_logged_check.notify_all();
            }
        }
    }
    while(command_blocks_logged.size()>0){
            std::string command;
            command_blocks_mutex.lock();
            command = command_blocks_logged.front();
            command_blocks_logged.pop();
            command_blocks_mutex.unlock();
            std::string filename = "bulk" + std::to_string(first_command_time) + thread_id + std::to_string(rand()) + ".log";
            std::ofstream file_out(filename);
            file_out << command;
            file_out.close();
            if(command_blocks_logged.size()>0){
                queue_logged_check.notify_all();
            }
        }

}

}