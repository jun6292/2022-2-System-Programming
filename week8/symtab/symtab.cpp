#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

int main()
{
    string line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream file("SRCFILE"); // SRCFILE 파일을 읽어온다.
    // SRCFILE의 symtab 자료구조 - hash table, label을 key로 label에 대한 주소를 value로 설정
    unordered_map<string, unsigned long long> symtab;
    vector<string> label_vec;   // label을 순서대로 출력하기 위한 벡터
    vector<int> flag_vec;   // flag를 저장하는 벡터
    unsigned long long loc = 0;    // LOCCTR, 주소를 배정하기 위한 16진수 변수
    unsigned long long addr = loc;

    while (getline(file, line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(line);
        ss.str(line);

        string label, opcode, operand;
        ss >> label >> opcode >> operand;   // label, opcode, operand 공백 기준으로 자른다.

        if (operand.empty()) {  // label이 없는 경우를 처리
            operand = opcode;
            opcode = label;
            label = "-";
        }

        if (opcode == "start") {    // LOCCTR을 시작 주소로 초기화
            loc = stoull(operand, NULL, 16);    // 문자열을 16진수로 변환    
            addr = loc;
        }
        else {
            if (opcode == "word")   // opcode가 word일 때 처리
                loc = addr + 3;
            else if (opcode == "resw")  // opcode가 resw일 때 처리
                loc = addr + 3 * stoull(operand);
            else if (opcode == "resb")  // opcode가 resb일 때 처리
                loc = addr + stoull(operand);
            else if (opcode == "byte")  // opcode가 byte일 때 처리
                loc = addr + operand.length() - 3; // c'' 제외 
            else if (opcode != "start")  // 나머지 처리
                loc = addr + 3;
        }
        if (label != "-" && opcode != "start") {
            symtab.insert({label, addr});
            label_vec.push_back(label);
            flag_vec.push_back(0);
        }
        addr = loc;
    }
    // 중복되는 label 발견 시 flag 처리
    for (int i = 0; i < label_vec.size(); i++) {
        for (int j = i + 1; j < label_vec.size(); j++) {
            if (label_vec[i] == label_vec[j]) {
                flag_vec[i] = 1;
                flag_vec[j] = 1;
            }
        }
    }
    // symtab 출력
    for (int i = 0; i < label_vec.size(); i++) {
        cout << "Label: " << label_vec[i] << ", ";
        cout << "Loc: " << std::hex << symtab[label_vec[i]] << ", ";
        cout << "flag: " << flag_vec[i] << "\n";
    }
}