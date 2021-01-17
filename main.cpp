#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/wait.h>

using namespace std;

//TODO history command and select command
//TODO write in file
// TODO Coloring
// TODO sudo
// TODO path next to shell
// TODO username next to shell

void exit_with_message(const string &message, int exit_status) {
    write(STDERR_FILENO, message.c_str(), message.length());
    exit(exit_status);
}

void write_stdout(string &message) {
    write(STDOUT_FILENO, message.c_str(), message.length());
}

void write_shell_prefix() {
    string prefix = "shell> ";
    write_stdout(prefix);
}

string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
    return s.erase(s.find_last_not_of(t) + 1);;
}

string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
    return s.erase(0, s.find_first_not_of(t));;
}

string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
    return ltrim(rtrim(s, t), t);
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

int foreground_process(vector<char *> args) {
    return execvp(args[0], &args[0]);
}

void execute_commands(const vector<string> &commands) {
    for (string command:commands) {
        vector<string> tokenize_command = tokenize_string(command, " ");
        vector<char *> arguments;
        if (tokenize_command.size() > 1) {
            arguments.reserve(tokenize_command.size() - 1);
        }
        for (const string &token : tokenize_command) {
            arguments.push_back(const_cast<char *>(token.c_str()));
        }
        arguments.push_back(NULL);
        string file = tokenize_command[0];
        if (file == "cd") {
            if (arguments.size() > 1 && arguments[1]) {
                chdir(arguments[1]);
//                TODO check result
            } else {
                chdir(getenv("HOME"));
            }
        } else if (file == "pwd") {
            char temp[1000];
            getcwd(temp, 1000);
            string message = string(temp);
            message += "\n";
            write_stdout(message);
        } else if (file == "exit") {
            exit(0);
        } else {
            int pid = fork();
            if (pid < 0) {
                exit_with_message("Error: Fork failed!", 1);
            } else if (pid == 0) {
                foreground_process(arguments);
            } else {
                while (1) {
                    pid_t done = wait(NULL);
                    if (done == -1) {
                        break;
                    }
                }

            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        string error_message = "An error has occurred\n";
        exit_with_message(error_message, 1);
    }
    istream &input_stream(std::cin);
    string line;

    while (!input_stream.eof()) {
        write_shell_prefix();
        getline(input_stream, line);
        vector<string> commands = tokenize_string(line, "&&");
        execute_commands(commands);
    }
    return 0;

}
