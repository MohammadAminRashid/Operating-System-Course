#include <iostream>
#include <vector>
#include <string>
using namespace std;

vector<string> string_splitter(const string& command_line, char splitter)
{
    vector<string> words;
    string word;

    for (char c : command_line) {
        if (c == splitter) {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


int main() {
    vector<string> a=string_splitter("FLIGHT_LIST",' ');
    cout<<a[0];
    return 0;
}
