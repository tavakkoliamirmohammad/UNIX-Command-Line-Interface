#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

void exit_with_message(string &message, int exit_status) {
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
        line.erase(0, pos + delimiter.length());
    }
    commands.push_back(trim(line));
    return commands;
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
    }
    return 0;

}
