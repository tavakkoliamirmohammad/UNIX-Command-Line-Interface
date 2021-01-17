#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/wait.h>

using namespace std;

// TODO history command and select command
// TODO write in file
// TODO Coloring
// TODO sudo
// TODO path next to shell
// TODO username next to shell
// TODO clear shell
// TODO fix string space bug
// TODO arrow key

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

void write_shell_prefix() {
    string prefix = "shell> ";
    write_stdout(prefix);
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
        exit(1);
    } else {
        waitpid(pid, &status, WUNTRACED);
        int child_return_code = WEXITSTATUS(status);
        if (child_return_code != 0) {
            exit_with_message("Error: failed", 2);
        }
    }
}

void background_process(vector<char *> args, int &background_process, int maximum_background_process) {
    if (background_process == maximum_background_process) {
        write_stderr("Error: Maximum number of background processes\n");
        return;
    }
    ++background_process;
    int pid = fork();
    if (pid < 0) {
        exit_with_message("Error: Fork failed!", 1);
    } else if (pid == 0) {
        execvp(args[1], &args[1]);
        exit(1);
    } else {
        write_stdout("Background process with " + to_string(pid) + " Executing\n");
    }
}

void check_background_process_finished(int &background_process_number) {
    int status = waitpid(-1, nullptr, WNOHANG);
    if (status > 0) {
        write_stdout("Background process with " + to_string(status) + " finished\n");
        background_process_number--;
    }
}

void execute_commands(const vector<string> &commands, int &background_process_number, int maximum_background_process) {
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
        } else if (file == "bg") {
            background_process(arguments, background_process_number, maximum_background_process);
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
    string line;

    int background_process_number = 0;
    int maximum_background_process = 5;
    while (!input_stream.eof()) {
        check_background_process_finished(background_process_number);
        write_shell_prefix();
        getline(input_stream, line);
        vector<string> commands = tokenize_string(line, "&&");
        execute_commands(commands, background_process_number, maximum_background_process);
    }
    return 0;

}
