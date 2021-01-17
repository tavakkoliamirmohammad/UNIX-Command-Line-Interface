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
#include <csignal>

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

void show_error_command(const vector<char *> &args) {
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
    int status;
    pid_t pid_finished = waitpid(-1, &status, WNOHANG | WCONTINUED | WUNTRACED);
    if (pid_finished > 0) {
        if (WIFCONTINUED(status)) {
            write_stdout("Background process with " + to_string(pid_finished) + " Continued\n");
        }
        else if (WIFSTOPPED(status)) {
            write_stdout("Background process with " + to_string(pid_finished) + " Stopped\n");
        }
        else if (WIFSIGNALED(status)) {
            background_processes.erase(pid_finished);
            write_stdout("Background process with " + to_string(pid_finished) + " finished\n");
        }

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

void background_process_signal(pid_t pid, int signal) {
    int res = kill(pid, signal);
    if (res == -1) {
        write_stderr(strerror(errno));
        write_stderr("\n");
    }
}

pid_t get_nth_background_process(unordered_map<pid_t, string> &background_processes, int n) {
    int i = 1;
    for (auto &process : background_processes) {
        if (i == n) {
            return process.first;
        }
        ++i;
    }
    return -1;
}

void change_directory(vector<char *> args) {
    if (args.size() > 1 && args[1]) {
        int res = chdir(args[1]);
        if (res == -1) {
            show_error_command(args);
        }
    } else {
        int res = chdir(getenv("HOME"));
        if (res == -1) {
            show_error_command(args);
        }
    }
}

void show_current_directory(vector<char *> args) {
    char temp[MAX_SIZE];
    char *res;
    res = getcwd(temp, MAX_SIZE);
    if (res != nullptr) {
        string message = string(temp);
        message += "\n";
        write_stdout(message);
    } else {
        show_error_command(args);
    }
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
            change_directory(arguments);
        } else if (file == "pwd") {
            show_current_directory(arguments);
        } else if (file == "exit") {
            exit(0);
        } else if (file == "bglist") {
            show_background_process(background_processes);
        } else if (file == "bgkill") {
            pid_t pid = get_nth_background_process(background_processes, stoi(arguments[1]));
            if (pid == -1) {
                stringstream ss;
                ss << file << ": " << "Invalid n number" << endl;
                write_stderr(ss.str());
                return;
            }
            background_process_signal(pid, SIGTERM);

        } else if (file == "bgstop") {
            pid_t pid = get_nth_background_process(background_processes, stoi(arguments[1]));
            if (pid == -1) {
                stringstream ss;
                ss << file << ": " << "Invalid n number" << endl;
                write_stderr(ss.str());
                return;
            }
            background_process_signal(pid, SIGSTOP);

        } else if (file == "bgstart") {
            pid_t pid = get_nth_background_process(background_processes, stoi(arguments[1]));
            if (pid == -1) {
                stringstream ss;
                ss << file << ": " << "Invalid n number" << endl;
                write_stderr(ss.str());
                return;
            }
            background_process_signal(pid, SIGCONT);

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
    int maximum_background_process = 5;
    while (!input_stream.eof()) {
        line = readline(write_shell_prefix().c_str());
        if (*line) add_history(line);
//        getline(input_stream, line);
        check_background_process_finished(background_processes);
        vector<string> commands = tokenize_string(line, "&&");
        execute_commands(commands, background_processes, maximum_background_process);
        free(line);
    }
    return 0;

}
