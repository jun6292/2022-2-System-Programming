#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <vector>
#include <unordered_map>

using namespace std;

int main()
{
    string line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream file("SRCFILE"); // SRCFILE 파일을 읽어온다.
    ofstream out("INTFILE");    // INTFILE로 출력한다.
    unordered_map<unsigned long long, string> file_map;
    vector<string> label_vec;   // label을 저장
    vector<unsigned long long> loc_vec; // 주소를 저장
    unsigned long long loc = 0;    // LOCCTR, 주소를 배정하기 위한 16진수 변수
    unsigned long long addr = loc;

    while (getline(file, line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(line);
        ss.str(line);

        string label, opcode, operand;
        ss >> label >> opcode >> operand;   // label, opcode, operand 공백 기준으로 자른다.
        // label이 없는 경우를 처리
        if (operand.empty()) {
            operand = opcode;
            opcode = label;
            label = "-";
        }
        // rsub와 같이 label과 operand가 없는 경우 처리
        if (opcode.empty() && operand.empty()) {
            opcode = label;
            label = "-";
            operand = "-";
        }
        label_vec.push_back(label); // label 벡터에 label을 저장
        if (opcode == "start") {    // LOCCTR을 시작 주소로 초기화
            loc = stoull(operand, NULL, 16);    // 문자열을 16진수로 변환    
            addr = loc;
            out << std::hex << addr << "  " << line << '\n';    // 첫째줄 출력
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
            else  // 나머지 처리
                loc = addr + 3;
        }
        if (opcode != "start") {    
            loc_vec.push_back(addr);    // lec_vec에 주소 배정 결과를 저장
            file_map.insert({addr, line});  // 출력을 위해 file_map에 주소를 key로 line을 value로 저장
        }
        addr = loc; // 주소 계산
    }
    int file_lines = loc_vec.size();
    for (int i = 0; i < file_lines; i++) {  // INTFILE에 출력
        out << std::hex << loc_vec[i] << "  ";
        out << file_map[loc_vec[i]] << '\n';
    }
}