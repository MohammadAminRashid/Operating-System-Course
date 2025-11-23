// src/main.cpp
// Mini-shell for OS-CA2 project
// Supports builtins: echo, print_n, len
// Supports unnamed pipes via pipe()/fork()/dup2()
// Supports simple named-pipe redirection to/from a FIFO called "np"
// Builtins are executed inside child processes to demonstrate fork/pipe behavior

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

vector<string> split_tokens(const string &s)
{
    vector<string> out;
    string current_string;
    bool in_quote = false;
    for (int i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (c == '"')
        {
            in_quote = !in_quote;
            continue;
        }
        if (isspace((unsigned char)c) and !in_quote)
        {
            if (!current_string.empty())
            {
                out.push_back(current_string);
                current_string = "";
            }
        }
        else
        {
            current_string.push_back(c);
        }
    }
    if (!current_string.empty())
        out.push_back(current_string);
    return out;
}

vector<string> split_pipe(const string &line)
{
    vector<string> parts;
    string current_string;
    bool in_quote = false;
    for (int i = 0; i < line.size(); i++)
    {
        char c = line[i];
        if (c == '"')
        {
            in_quote = !in_quote;
            current_string.push_back(c);
            continue;
        }
        if (c == '|' && !in_quote)
        {
            parts.push_back(trim(current_string));
            current_string = "";
        }
        else
            current_string.push_back(c);
    }

    string last_word = trim(current_string);
    if (!last_word.empty())
    {
        parts.push_back(last_word);
    }

    return parts;
}

// Execute builtin in current process context (reads/writes stdio).
void run_builtin(const vector<string> &args)
{
    if (args.empty())
        return;
    string cmd = args[0];
    if (cmd == "echo")
    {
        if (args.size() > 1)
        {
            for ( int i = 1; i < args.size(); ++i)
            {
                if (i > 1)
                    cout << " ";
                cout << args[i];
            }
            cout << endl;
        }
        else
        {
            string line;
            bool first = true;
            while (getline(cin, line))
            {
                if (!first)
                    cout << "\n";
                cout << line;
                first = false;
            }
            cout << flush;
        }
    }
    else if (cmd == "print_n")
    {
        if (args.size() < 2)
        {
            cerr << "print_n requires a numeric argument\n";
            return;
        }
        int n = atoi(args[1].c_str());
        for (int i = 0; i < n; i++)
        {
            if (i)
                cout << ", ";
            cout << "hello";
        }
        cout << endl;
    }
    else if (cmd == "len")
    {
        if (args.size() > 1)
        {
            string s;
            for (size_t i = 1; i < args.size(); ++i)
            {
                if (i > 1)
                    s.push_back(' ');
                s += args[i];
            }
            cout << s.size() << endl;
        }
        else
        {
            string all;
            string line;
            bool first = true;
            while (getline(cin, line))
            {
                if (!first)
                    all.push_back('\n');
                all += line;
                first = false;
            }
            cout << all.size() << endl;
        }
    }
    else
    {
        cerr << "Unknown builtin: " << cmd << "\n";
    }
    fflush(stdout);
}

int open_named_pipe_read(const string &name)
{
    int fd = open(name.c_str(), O_RDONLY);
    return fd;
}
int open_named_pipe_write(const string &name)
{
    int fd = open(name.c_str(), O_WRONLY);
    return fd;
}

int main()
{

    const string named_pipe = "np";

    string line;
    while (1)
    {

        if (!getline(cin, line))
        {
            break;
        }
        if (line.empty())
        {
            continue;
        }

        string tmp = trim(line);

        if (tmp == "exit")
            break;

        bool redirect_in_np = false, redirect_out_np = false;
        vector<string> tokens = split_tokens(tmp);
        vector<string> cleaned_tokens;
        for (int i = 0; i < tokens.size(); i++)
        {
            if ((i + 1) < tokens.size() and tokens[i + 1] == "np")
            {
                if (tokens[i] == "<")
                {
                    redirect_in_np = true;
                    i++;
                    continue;
                }
                else if (tokens[i] == ">")
                {
                    redirect_out_np = true;
                    i++;
                    continue;
                }
            }

            else
            {
                cleaned_tokens.push_back(tokens[i]);
            }
        }

        string cleaned_line;
        for (int i = 0; i < cleaned_tokens.size(); i++)
        {

            cleaned_line += cleaned_tokens[i];
            if (i == cleaned_tokens.size() - 1)
            {
                break;
            }
            cleaned_line.push_back(' ');
        }

        if (cleaned_line.empty())
        {
            continue;
        }
        // cout << cleaned_line << endl;
        vector<string> parts = split_pipe(cleaned_line);
        // for (auto w : parts){

        //     cout<<w<<"qqq";
        // }
        int n = parts.size();
        vector<int> pfd_read(n - 1, -1), pfd_write(n - 1, -1);
        vector<pid_t> children;

        for (int i = 0; i < n - 1; ++i)
        {
            int fds[2];
            if (pipe(fds) == -1)
            {
                perror("pipe");
            }
            pfd_read[i] = fds[0];
            pfd_write[i] = fds[1];
        }

        for (int i = 0; i < n; ++i)
        {
            vector<string> args = split_tokens(parts[i]);
            // for (auto a :  args ){

            //     cout<<a<<endl;
            // }
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork");
                break;
            }
            else if (pid == 0)
            {
                if (i == 0)
                {
                    if (redirect_in_np)
                    {
                        int fd = open_named_pipe_read(named_pipe);
                        if (fd == -1)
                        {
                            perror("open np for read");
                            _exit(1);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                }
                else
                {
                    dup2(pfd_read[i - 1], STDIN_FILENO);
                }

                if (i == n - 1)
                {
                    if (redirect_out_np && n == 1)
                    {
                        int fd = open_named_pipe_write(named_pipe);
                        if (fd == -1)
                        {
                            perror("open np for write");
                            _exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                }
                else
                {
                    dup2(pfd_write[i], STDOUT_FILENO);
                }
                // Close all parent pipe fds in child
                for (int j = 0; j < n - 1; ++j)
                {
                    if (pfd_read[j] != -1)
                        close(pfd_read[j]);
                    if (pfd_write[j] != -1)
                        close(pfd_write[j]);
                }
                // Execute builtin
                run_builtin(args);
                _exit(0);
            }
            else
            {
                children.push_back(pid);
            }
        }

        for (int j = 0; j < n - 1; ++j)
        {
            if (pfd_read[j] != -1)
                close(pfd_read[j]);
            if (pfd_write[j] != -1)
                close(pfd_write[j]);
        }
        for (pid_t c : children)
        {
            int status;
            waitpid(c, &status, 0);
        }
    }

    return 0;
}
