#ifndef CONTEXT_H
#define CONTEXT_H

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <queue>
#include <ctime>
#include <condition_variable>
#include <random>


namespace async{

    class Context
    {
    private:
        std::thread log_thread;
        std::thread file_thread_1;
        std::thread file_thread_2;

        std::string last_command;
        std::vector<std::string> commands;
        std::vector<std::string> current_block;
        std::queue<std::string> command_blocks;
        std::queue<std::string> command_blocks_logged;
    
        std::condition_variable command_check;
        std::condition_variable queue_logged_check;
        std::mutex command_lock;
        std::mutex query_lock;
        std::mutex command_blocks_mutex;

        int bracket_counter;
        int bulk_size;
        time_t first_command_time;
        bool finish_flag;
    public:
        Context(size_t bulk_size);
        ~Context();
        void write_commands(std::string);
        void log_commands();
        void add_commands(const char *data, std::size_t size);
        void process_commands();
        void group_command(std::vector<std::string> v);
        void disconnect();

    };


}

#endif