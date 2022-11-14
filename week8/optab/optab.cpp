#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace std;

int main() 
{
    string line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream file("optab.txt"); // "optab.txt" 파일을 읽어온다.
    unordered_map<string, string> op_map;   // 해시맵 선언

    while (getline(file, line)) {   // 파일을 한 줄씩 읽어온다.
        cout << line << '\n';
        stringstream ss(line);
        ss.str(line);

        string inst, code;  // 명령어와 명령어에 대한 코드
        ss >> inst >> code;   // 명령어와 명령어에 대한 코드를 공백 기준으로 자른다.
        op_map.insert({inst, code});    // 명령어를 key로 명령어에 대한 코드를 value로 해시맵이 저장한다.
    }

    string input;   
    while (true) {
        cout << "Input instruction(exit: q): ";  // 사용자로부터 instruction 입력을 받는다.
        cin >> input;
        // input을 대문자로 바꿔준다.
        transform(input.begin(), input.end(), input.begin(), [](unsigned char c){ return toupper(c); });
        if (input == "Q")   // 사용자가 q를 입력하면 반복 종료
            break;
        if (op_map.find(input) != op_map.end()) // 해당 명령어가 존재하면 그에 대한 코드 출력
            cout << op_map[input] << '\n';
        else
            cout << "Error: Instruction not found\n";   // 해당 명령어가 없다면 에러 메시지 출력, 입력을 다시 받는다.
    }
}