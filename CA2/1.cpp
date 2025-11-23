#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NAMEPIPE "np"

using namespace std;

// متغیر global برای مدیریت بهتر بسته شدن
int fifo_fd = -1;

void setup_fifo()
{
    if (mkfifo(NAMEPIPE, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
        }
    }
    fifo_fd = open(NAMEPIPE, O_RDWR);
    if (fifo_fd == -1)
    {
        perror("open fifo global");
        exit(1);
    }
}

string read_from_fifo()
{
    if (fifo_fd == -1)
        return "";

    string line = "";
    char c;

    while (read(fifo_fd, &c, 1) > 0)
    {
        if (c == '\n')
            break;
        line.push_back(c);
    }
    return line;
}

void write_to_fifo(const string &msg)
{
    if (fifo_fd == -1)
        return;

    string final_msg = msg + "\n";

    if (write(fifo_fd, final_msg.c_str(), final_msg.size()) == -1)
    {
        perror("write to fifo");
    }
}

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

string exec_command(string command)
{
    string out = "";
    vector<string> args = split_tokens(command);
    if (args.size() < 1)
    {
        return "";
    }

    if (args[0] == "echo")
    {
        for (int i = 1; i < args.size(); i++)
        {
            out += args[i];
            if (i != args.size() - 1)
            {
                out += ' ';
            }
        }
        return out;
    }
    else if (args[0] == "len")
    {
        for (int i = 1; i < args.size(); i++)
        {
            out += args[i];
            if (i != args.size() - 1)
            {
                out += ' ';
            }
        }
        return to_string(out.size());
    }
    else if (args[0] == "print_n")
    {
        int n = stoi(args[1]);
        for (int i = 0; i < n; i++)
        {
            if (i == n - 1)
            {
                out += "hello";
                break;
            }
            out += "hello, ";
        }
        return out;
    }
    // نکته: exit را اینجا برداشتیم چون در main مدیریت می‌شود،
    // اما بودن آن هم مشکلی ایجاد نمی‌کند اگر در main چک شود.
    else
    {
        // برای جلوگیری از چاپ unknown command برای دستورات خالی یا خاص
        if(args[0] != "exit") 
            cout << "unknown command: " << args[0] << endl;
        return "";
    }

    return out;
}

int main()
{
    setup_fifo();

    string line;
    while (1)
    {
        // چاپ پرامپت برای مشخص شدن وضعیت
        cout << "> "; 
        if (!getline(cin, line))
        {
            break;
        }
        if (line.empty())
        {
            continue;
        }

        string tmp = trim(line);

        // --- اصلاحیه شروع شد ---
        // بررسی می‌کنیم آیا کاربر درخواست خروج داده است؟
        // این کار باید قبل از هرگونه پردازش پایپ یا فورک انجام شود.
        if (tmp == "exit") 
        {
            if (fifo_fd != -1)
            {
                close(fifo_fd);
            }
            cout << "Exiting shell..." << endl;
            break; // خروج از حلقه اصلی و پایان برنامه
        }
        // --- اصلاحیه تمام شد ---

        bool redirect_in_np = false, redirect_out_np = false;

        vector<string> tokens = split_tokens(tmp);
        vector<string> cleaned_tokens;
        for (int i = 0; i < tokens.size(); i++)
        {
            if ((i + 1) < tokens.size() and tokens[i + 1] == NAMEPIPE)
            {
                if (tokens[i] == "<" and i == 1)
                {
                    redirect_in_np = true;
                    i++;
                    continue;
                }
                else if (tokens[i] == ">" and i == tokens.size() - 2)
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

        vector<string> commands = split_pipe(cleaned_line);

        // بررسی مجدد برای حالتی که exit با فاصله تایپ شده باشد یا بخشی از یک پایپ نباشد (اختیاری)
        if (commands.size() > 0 && split_tokens(commands[0])[0] == "exit") {
             if (fifo_fd != -1) close(fifo_fd);
             break;
        }

        int n = commands.size();
        string command = commands[0];

        if (redirect_in_np)
        {
            string data_from_pipe = read_from_fifo();
            command += " " + data_from_pipe;
        }

        for (int i = 0; i < n; i++)
        {
            int fd[2];
            if (pipe(fd) == -1)
            {
                perror("pipe");
                exit(1);
            }
            pid_t pid = fork();

            if (pid > 0)
            {
                close(fd[0]);
                string out = exec_command(command);
                if (i == n - 1)
                {
                    close(fd[1]);
                    if (redirect_out_np)
                    {
                        write_to_fifo(out);
                    }
                    else
                    {
                        // فقط اگر خروجی خالی نبود چاپ کن تا خط خالی اضافی ندهد
                        if(!out.empty()) cout << out << endl;
                    }
                }
                else 
                {
                    write(fd[1], out.c_str(), out.size() + 1); // +1 برای کاراکتر null
                    close(fd[1]);
                }

                wait(NULL);
                if (i == 0)
                {
                    break; // خروج والد از حلقه فورک برای بازگشت به دریافت ورودی
                }
                else
                {
                    _exit(0); // پروسه‌های میانی باید بسته شوند
                }
            }
            else if (pid == 0)
            {
                close(fd[1]);
                char buf[4096];
                ssize_t nread = read(fd[0], buf, sizeof(buf));
                close(fd[0]);
                if (nread <= 0)
                {
                    // اگر والد چیزی ننوشت یا اروری بود، فرزند نباید ادامه دهد
                    exit(0); 
                }
                buf[nread] = '\0'; 
                // نکته مهم: وقتی داده باینری نیست و string است، باید مراقب null terminator باشید
                // اما چون write شما سایز + 1 می‌فرستد، اینجا امن است.
                
                string x = buf; // تبدیل بافر به رشته

                // دستور بعدی را آماده کن
                if (i + 1 < commands.size()) {
                    command = commands[i + 1] + " " + x;
                } else {
                    exit(0);
                }
            }
            else 
            {
                perror("fork failed");
            }
        }
    }
    return 0;
}