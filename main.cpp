#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/wait.h>
#include <regex>
#include "colors.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <unordered_map>

// TODO write in file
// TODO fix string space bug

#define MAX_SIZE 1024

void exit_with_message(const string &message, int exit_status) {
    write(STDERR_FILENO, message.c_str(), message.length());
    exit(exit_status);
}

void write_stderr(const string &message) {
    write(STDERR_FILENO, message.c_str(), message.length());
}

void write_stdout(const string &message) {
    write(STDOUT_FILENO, message.c_str(), message.length());
}

char hostname[MAX_SIZE];

string write_shell_prefix() {
    std::stringstream ss;
    gethostname(hostname, MAX_SIZE);
    ss << bold(red("\u21aa ")) << bold(green(getlogin())) << bold(cyan("@")) << bold(green(hostname)) << " "
       << bold(cyan(regex_replace(get_current_dir_name(), regex("/home/" + string(getlogin())), "~")))
       << yellow(" shell> ");
//    write_stdout(ss.str());
    return ss.str();
}

string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
    return s.erase(s.find_last_not_of(t) + 1);
}

string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
    return s.erase(0, s.find_first_not_of(t));
}

string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
    return ltrim(rtrim(s, t), t);
}

void show_error_command(vector<char *> args) {
    write_stderr(args[0]);
    write_stderr(": ");
    write_stderr(strerror(errno));
    write_stderr("\n");
}

vector<string> tokenize_string(string line, const string &delimiter) {
    size_t pos;
    std::string token;
    vector<string> commands;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        commands.push_back(trim(token));
        line = trim(line.erase(0, pos + delimiter.length()));
    }
    commands.push_back(trim(line));
    return commands;
}

void foreground_process(vector<char *> args) {
    int status;
    int pid = fork();
    if (pid < 0) {
        exit_with_message("Error: Fork failed!", 1);
    } else if (pid == 0) {
        execvp(args[0], &args[0]);
        show_error_command(args);
        exit(0);
    } else {
        waitpid(pid, &status, WUNTRACED);
        int child_return_code = WEXITSTATUS(status);
//        if (child_return_code != 0) {
//            exit_with_message("Error: failed", 2);
//        }
    }
}

void background_process(vector<char *> args, unordered_map<pid_t, string> &background_processes_list,
                        int maximum_background_process) {
    if (background_processes_list.size() == maximum_background_process) {
        write_stderr("Error: Maximum number of background processes\n");
        return;
    }
    int pid = fork();
    if (pid < 0) {
        exit_with_message("Error: Fork failed!", 1);
    } else if (pid == 0) {
        execvp(args[1], &args[1]);
        show_error_command(vector<char *>(args.begin() + 1, args.end()));
//        exit(1);
    } else {
        background_processes_list[pid] = args[1];
        write_stdout("Background process with " + to_string(pid) + " Executing\n");
    }
}

void check_background_process_finished(unordered_map<pid_t, string> &background_processes) {
    pid_t pid_finished = waitpid(-1, nullptr, WNOHANG);
    if (pid_finished > 0) {
        write_stdout("Background process with " + to_string(pid_finished) + " finished\n");
        background_processes.erase(pid_finished);
    }
}

void show_background_process(unordered_map<pid_t, string> &background_processes) {
    int i = 0;
    for (auto &process : background_processes) {
        stringstream ss;
        ++i;
        ss << "(" << i << ")" << " " << process.second << endl;
        write_stdout(ss.str());
    }
    stringstream ss;
    ss << "Total Background Jobs: " << i << endl;
    write_stdout(ss.str());
}

void execute_commands(const vector<string> &commands, unordered_map<pid_t, string> &background_processes,
                      int maximum_background_process) {
    for (const string &command:commands) {
        vector<string> tokenize_command = tokenize_string(command, " ");
        vector<char *> arguments;
        arguments.reserve(tokenize_command.size() + 1);
        for (const string &token : tokenize_command) {
            arguments.push_back(const_cast<char *>(token.c_str()));
        }
        arguments.push_back(nullptr);
        string file = arguments[0];
        if (file == "cd") {
            if (arguments.size() > 1 && arguments[1]) {
                chdir(arguments[1]);
                show_error_command(arguments);
//               TODO check result
            } else {
                chdir(getenv("HOME"));
            }
        } else if (file == "pwd") {
            char temp[MAX_SIZE];
            getcwd(temp, MAX_SIZE);
            string message = string(temp);
            message += "\n";
            write_stdout(message);
        } else if (file == "exit") {
            exit(0);
        } else if (file == "bglist") {
            show_background_process(background_processes);
        } else if (file == "bg") {
            background_process(arguments, background_processes, maximum_background_process);
        } else {
            foreground_process(arguments);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        string error_message = "An error has occurred\n";
        exit_with_message(error_message, 1);
    }
    istream &input_stream(std::cin);

    unordered_map<pid_t, string> background_processes;

    char *line;


    int background_process_number = 0;
    int maximum_background_process = 5;
    while (!input_stream.eof()) {
        check_background_process_finished(background_processes);

        line = readline(write_shell_prefix().c_str());
        if (*line) add_history(line);
//        getline(input_stream, line);
        vector<string> commands = tokenize_string(line, "&&");
        execute_commands(commands, background_processes, maximum_background_process);
        free(line);
    }
    return 0;

}
