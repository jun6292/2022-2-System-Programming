#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <vector>
#include <iomanip>
#include <unordered_map>

using namespace std;

unsigned long long ctoaskii(char c) {   // 문자를 아스키 코드로 변환
    unsigned long long i = c;
    return i;
}

int main()
{
    string op_line, src_line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream srcfile("SRCFILE"); // SRCFILE 파일을 읽어온다.
    ifstream opfile("optab.txt"); // "optab.txt" 파일을 읽어온다.
    unordered_map<string, string> op_map;   // optab을 저장할 해시 table
    unordered_map<unsigned long long, string> file_map;
    unordered_map<string, unsigned long long> symtab;   // symtab
    vector<string> label_vec;   // label을 저장
    vector<unsigned long long> loc_vec; // 배정된 주소를 저장
    vector<int> flag_vec;   // flag를 저장하는 벡터
    ofstream out("LISFILE");    // LISFILE로 출력한다.
    unsigned long long loc = 0;    // LOCCTR, 주소를 배정하기 위한 16진수 변수
    unsigned long long addr = loc;
    
    // optab 형성
    while (getline(opfile, op_line)) {   // optab.txt 파일을 한 줄씩 읽어온다.
        stringstream ss(op_line);
        ss.str(op_line);

        string inst, code;    // 명령어와 명령어에 대한 코드
        ss >> inst >> code;   // 명령어와 명령어에 대한 코드를 공백 기준으로 자른다.
        op_map.insert({inst, code});    // 명령어를 key로 명령어에 대한 코드를 value로 optab에 저장
    }
    opfile.close();

    // symtab 형성
    while (getline(srcfile, src_line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(src_line);
        ss.str(src_line);

        string label, opcode, operand;
        ss >> label >> opcode >> operand;   // label, opcode, operand 공백 기준으로 자른다.

        if (operand.empty()) {  // label이 없는 경우를 처리
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
                loc = addr + operand.length() - 3; // c'' 제외 , x'' 제외
            else  // 나머지 처리
                loc = addr + 3;
        }
        if (label != "-" && opcode != "start") {
            symtab.insert({label, addr});
            label_vec.push_back(label);
            flag_vec.push_back(0);
        }
        if (opcode != "start") {    
            loc_vec.push_back(addr);    // lec_vec에 주소 배정 결과를 저장
            file_map.insert({addr, src_line});  // 출력을 위해 file_map에 주소를 key로 line을 value로 저장
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
    
    // LISFILE 출력, object code 출력
    string lis_line;
    ifstream file("SRCFILE"); // SRCFILE 파일을 읽어온다.
    loc = 0;    // LOCCTR, 주소를 배정하기 위한 16진수 변수
    addr = loc;
    int i = 0;
    while (getline(file, lis_line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(lis_line);
        ss.str(lis_line);

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

        if (opcode == "start") {    // LOCCTR을 시작 주소로 초기화
            loc = stoull(operand, NULL, 16);    // 문자열을 16진수로 변환    
            addr = loc;
            out << hex << addr << "        " << lis_line << '\n';    // 프로그램명 start 시작주소 출력
        }
        else {
            out << hex << loc_vec[i] << " "; 
            i++;
        }
        unsigned long long objcode;
        if (opcode == "word") { // opcode가 word일 때 object code 처리
            objcode = stoull(operand);
            out << setfill('0') << setw(6) << hex << objcode << " ";
            out << lis_line << '\n';
        }
        else if (opcode == "byte") {    // opcode가 byte일 때 object code 처리
            string objcode = operand.substr(2, operand.size() - 3);
            bool new_line = false;  // 개행을 위한 bool 타입 변수
            if (objcode.size() > 3) // 문자열 상수의 크기가 3보다 크면
                new_line = true;
            if (operand[0] == 'x') { // x''부분을 그대로 출력
                out << setfill(' ') << setw(6) << left << objcode;
                out << " " << lis_line << '\n';
            }
            else if (operand[0] == 'c') {   // 문자열을 16진수 아스키 코드로 변환
                unsigned long long ascii;
                if (objcode.size() > 3) {   // 문자열 상수의 크기가 3보다 크면
                    for (int i = 0; i < 3; i++) {   // object code를 3바이트만 출력
                        ascii = ctoaskii(objcode[i]);
                        out << hex << ascii;
                    }
                    out << " ";
                    out << lis_line << '\n';    // 개행 후 3바이트씩 출력을 반복
                    int add = 3;
                    while (new_line) {
                        out << "    " << " ";
                        for (int i = add; i < add + 3; i++) {
                            if (i > objcode.size() - 1)
                                break;
                            ascii = ctoaskii(objcode[i]);
                            out << hex << ascii;
                        }
                        add += 3;
                        out << '\n';
                        if (add > objcode.size() - 1)
                            new_line = false;
                    }
                } else {    // 문자열 상수의 크기가 3보다 작다면
                    for (int i = 0; i < objcode.size(); i++) {
                        ascii = ctoaskii(objcode[i]);
                        out << hex << ascii;
                    }
                    out << setfill(' ') << setw(6 - objcode.size() * 2) << left << " ";    // 나머지를 공백으로 채운다
                    out << " " << lis_line << '\n';
                }
            }
        }
        else if (opcode == "resb") {    // opcode가 resb일 때 object code 처리
            out << "      " << " ";
            out << lis_line << '\n';
        }
        else if (opcode == "resw") {    // opcode가 resw일 때 object code 처리
            out << "      ";
            out << lis_line << '\n';
        }
        else if (opcode != "start") {
            transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c){ return toupper(c); }); // opcode를 대문자로 변환
            if (opcode == "LDCH") { // opcode가 ldch라면, 인덱스 주소 지정 방식 사용
                string symbol = operand.substr(0, operand.size() - 2);
                out << op_map[opcode] << hex << symtab[symbol] + 32768 << " ";  // 9번째 비트를 1로 설정
                out << lis_line << '\n';
            }
            else if (opcode == "STCH") {    // opcode가 stch라면, 인덱스 주소 지정 방식 사용
                string symbol = operand.substr(0, operand.size() - 2);
                out << op_map[opcode] << hex << symtab[symbol] + 32768 << " ";  // 9번째 비트를 1로 설정
                out << lis_line << '\n';
            }
            else if (opcode == "END") { // opcode가 end일 때 처리
                out << " " << "      " << lis_line << '\n';
            }
            else {
                if (op_map.find(opcode) != op_map.end() && symtab.find(operand) != symtab.end()) {    // optab에 명령어가 존재하면서 symtab에 심볼이 존재
                    transform(op_map[opcode].begin(), op_map[opcode].end(), op_map[opcode].begin(), [](unsigned char c){ return tolower(c); }); // opcode를 소문자로 변환
                    out << op_map[opcode] << hex << symtab[operand] << " " << lis_line << '\n';
                }
                else if (op_map.find(opcode) == op_map.end() && symtab.find(operand) != symtab.end()) { // optab에 명령어가 없고 symtab에 심볼이 존재
                    out << " " << "      " << lis_line << '\n';
                    out << " **** unrecognized operation code\n";
                }
                else if (symtab.find(operand) == symtab.end() && op_map.find(opcode) != op_map.end()) { // symtab에 심볼이 없고 optab에 명령어가 존재
                    out << " " << "      " << lis_line << '\n';
                    out << " **** unrecognized symbol in operand\n";
                }
                else {  // optab에 명령어가 없고 symtab에 심볼이 없음
                    out << " " << "      " << lis_line << '\n';
                    out << " **** unrecognized operation code\n";
                    out << " **** unrecognized symbol in operand\n";
                }
            }
        }
    }
}