#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <unistd.h>

using namespace std;
unordered_map<string, string> op_map;   // optab을 저장할 해시 table
unordered_map<string, unsigned long long> symtab;   // symbol과 그 주소를 저장할 symtab
vector<string> label_vec;   // label을 저장하는 벡터
vector<int> flag_vec;   // flag를 저장하는 벡터
vector<unsigned long long> addr_vec; // 주소 값을 저장하는 벡터
vector<string> line_vec;    // srcfile의 line을 저장하는 벡터
unsigned long long prog_length; // 프로그램 길이를 저장하는 변수
unordered_map<unsigned long long, string> file_map; // srcfile 각 라인의 주소와 명령어를 저장하는 해시 맵


void make_optab() { // optab 형성
    string op_line;
    ifstream opfile("optab.txt"); // "optab.txt" 파일을 읽어온다.
    while (getline(opfile, op_line)) {   // "optab.txt" 파일을 한 줄씩 읽어온다.
        stringstream ss(op_line);
        ss.str(op_line);    // stringstream을 문자열로 반환
        string inst, code;    // 명령어와 명령어에 대한 코드
        ss >> inst >> code;   // 명령어와 명령어에 대한 코드를 공백 기준으로 자른다.
        op_map.insert({inst, code});    // 명령어를 key로 명령어에 대한 코드를 value로 optab에 저장
    }
    opfile.close();
}

void error(int flag) {  // 프로그램 에러 발생 시 처리
    if (flag == 1)
        cout << "Duplicate symbol in symtab\n"; // symtab에 중복되는 label이 있을 때
    else if (flag == 0)
        cout << "Not found in optab\n"; // optab에 존재하지 않는 명령어일 때
    else
        cout << "Illegal expression\n"; // 그 외 에러처리
    // exit(1);
}

string strtoupper(string str) { // string을 대문자로 변환
    string tmp;
    for (int i = 0; i < str.size(); i++)
        str[i] = toupper(str[i]);
    tmp = str;
    return tmp;
}

string strtoaskii(string str) {   // BYTE의 c' '를 처리
    ostringstream ss;
    unsigned long long ull; string tmp;
    for (int i = 0; i < str.size(); i++) {     // 문자열을 16진수 아스키 코드로 이루어진 문자열로 변환
        ull = str[i]; 
        ss << hex << ull;
        tmp = ss.str();
    }
    return tmp;
}
// WORD의 operand를 처리
string inttostr(string str) {   // WORD의 operand는 10진수로 입력된다.
    ostringstream ss;   
    int deci = stoi(str); string tmp;
    ss << hex << deci; tmp = ss.str();  // 10진수를 16진수로 변환한 후 문자열로 변환
    for (int i = tmp.size(); i < 6; i++) {
        tmp = "0" + tmp;    // 빈 공간을 0으로 채움.
    }
    return tmp;
}
// symbol의 주소(ull)를 16진수로 변환 후 string으로 반환
string ulltohex(unsigned long long ull) {   
    ostringstream ss;  ss << hex << ull;
    string tmp = ss.str();
    return tmp;
}
// 바이트로 나타낸 텍스트 레코드의 길이
string record_size(int rec_length) { // 레코드의 길이를 16진수로 변환하여 문자열로 반환
    ostringstream ss; ss << setfill('0') << setw(2) << right << hex << rec_length;
    string tmp = ss.str();
    return tmp;
}

unsigned long long strtoull_addr(string str) {   // 16진수 문자열 주소를 ull형으로 변환하여 반환
    unsigned long long num = 0;
    for (int i = 0; i < str.size(); i++) {
        if ('0' <= str[i] && str[i] <= '9')
            num = num * 16 + (str[i] - '0');
        else if ('A' <= str[i] && str[i] <= 'F')
            num = num * 16 + (str[i] - 'A' + 10);
    }
    return num;
}

void pass1(string file_name) {    // symtab을 형성하고 프로그램 길이를 반환
    string src_line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream srcfile(file_name); // 입력받은 파일을 읽어온다.
    unsigned long long loc = 0;    // LOCCTR, 주소를 배정하기 위한 16진수 변수
    unsigned long long addr = loc;
    unsigned long long sta_adr, end_adr;    // 시작 주소와 끝 주소
    // symtab 형성
    while (getline(srcfile, src_line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(src_line);
        ss.str(src_line);
        // label, opcode, operand 공백 기준으로 자른다.
        string label, opcode, operand;
        ss >> label >> opcode >> operand;
        // label이 없는 경우를 처리
        if (operand.empty()) {  
            operand = opcode; opcode = label; label = "-";
        }
        // rsub와 같이 label과 operand가 없는 경우 처리
        if (opcode.empty() && operand.empty()) {
            opcode = label; label = "-"; operand = "-";
        }
        // LOCCTR을 시작 주소로 초기화
        if (opcode == "start") {    
            loc = stoull(operand, NULL, 16);    // 문자열을 16진수로 변환    
            addr = loc; sta_adr = addr;
        }
        else {
            if (opcode == "word")   loc = addr + 3; // opcode가 word일 때 처리
            else if (opcode == "resw")  loc = addr + 3 * stoull(operand); // opcode가 resw일 때 처리
            else if (opcode == "resb")  loc = addr + stoull(operand); // opcode가 resb일 때 처리
            else if (opcode == "byte")  loc = addr + operand.length() - 3; // opcode가 byte일 때 처리, c'' 제외 + x'' 제외
            else    loc = addr + 3;  // 나머지 처리
        }
        if (label != "-" && opcode != "start") {
            symtab.insert({label, addr});   // symtab에 label과 해당 주소를 삽입
            label_vec.push_back(label);     // label 벡터에 label 삽입
            flag_vec.push_back(0);      // symtab의 error flag 설정을 위한 flag 벡터
        }
        if (opcode != "start") {
            addr_vec.push_back(addr); // loc_vec에 주소를 순서대로 삽입
            file_map.insert({addr, src_line});  // 출력을 위해 file_map에 주소를 key로 line을 value로 저장
            line_vec.push_back(src_line);
        }
        if (opcode == "end")    // opcode가 end일 때, end_adr에 끝 주소 저장
            end_adr = addr;
        addr = loc;
        // opcode가 존재하지 않는 명령어라면 오류 처리
        if (opcode != "start" && opcode != "end") {
            if (opcode != "byte" && opcode != "word" && opcode != "resb" && opcode != "resw") {
                opcode = strtoupper(opcode);
                if (op_map.find(opcode) == op_map.end())    // opcode가 optab에 없다면 에러
                    error(0);
            }
        }
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
    for (int f : flag_vec) {    // 중복된 label이 있을 때
        if (f == 1)
            error(f);    // 오류 처리
    }
    prog_length = end_adr - sta_adr;    // 프로그램의 길이
}

void pass2(string file_name) {  // object code 출력
    ofstream out("OBJFILE");    // OBJFILE로 출력한다.
    string line, record, byte;    // line: 한 줄씩 파일을 읽는다, record: T 레코드의 목적코드를 저장, byte: 16진수 상수
    ifstream file(file_name); // 입력받은 file을 읽어온다.
    unsigned long long loc = 0;    // LOCCTR
    unsigned long long str_adr = 0; // 시작 주소
    int i = -1, rec_flag = 0;  // 파일의 현재 라인과 record의 상태
    bool newline = false;    // 텍스트 레코드의 줄 바꿈을 해야되는지 판단

    while (getline(file, line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(line);
        ss.str(line);
        // label, opcode, operand 공백 기준으로 자른다.
        string label, opcode, operand;
        ss >> label >> opcode >> operand;
        // label이 없는 경우를 처리
        if (operand.empty()) {
            operand = opcode; opcode = label; label = "-";
        }
        // rsub와 같이 label과 operand가 없는 경우 처리
        if (opcode.empty() && operand.empty()) {
            opcode = label; label = "-"; operand = "-";
        }
        opcode = strtoupper(opcode);    // opcode를 대문자로 변환
        if (opcode != "START" && (symtab.find(operand) == symtab.end())) {
            if (opcode != "WORD" && opcode != "BYTE" && opcode != "RESB" && opcode != "RESW") {
                if (opcode != "RSUB" && opcode != "LDCH" && opcode != "STCH") {
                    out << setfill('0') << setw(6) << "0"; // 피연산자 주소를 0으로 설정
                    error(-1);
                }
            }        
        }   // symtab에 operand가 존재하지 않을 때
        if (opcode == "START") {    // 헤더 레코드 출력
            i++;
            loc = stoull(operand, NULL, 16);    // 문자열을 16진수로 변환    
            str_adr = loc;  // 프로그램 시작 주소 저장
            label = strtoupper(label);  // 프로그램 이름을 대문자로 변환
            out << "H" << setfill(' ') << setw(6) << left << label;
            out << setfill('0') << setw(6) << right << strtoupper(ulltohex(str_adr));
            out << setfill('0') << setw(6) << right << strtoupper(ulltohex(prog_length)) << '\n';
            // H(1), 프로그램 이름(2~7), 프로그램 시작주소(8~13), 프로그램 길이(14~19)
        }
        else if (opcode == "END") {     // 엔드 레코드 출력
            if (record.length() < 60) {    // 텍스트 레코드가 한 줄짜리 프로그램일 때
                record = record_size(record.length() / 2) + record;  // 바이트로 나타낸 레코드의 길이(16진수)와 16진수로 나타낸 목적 코드를 연결
                out << strtoupper(record) << '\n';  // 텍스트 레코드를 출력
            }
            out << "E" << setfill('0') << setw(6) << right << strtoupper(ulltohex(str_adr)) << '\n';
            // E(1), 목적 프로그램 중 첫번째로 실행될 명령어의 주소(2~7)
        }
        else {  // 텍스트 레코드 출력: T(1), 레코드에 포함될 시작 주소(2~7), 해당 레코드의 길이(8~9), 16진수 목적 코드(10~69)
            if (rec_flag == 0 && opcode != "RESB" && opcode != "RESW") {  // resb나 resw는 출력하지 않음
                out << "T"; rec_flag = 1;   // 레코드 길이가 0일 때 T 출력
            }
            if (rec_flag == 1) {  // 레코드 길이가 1일 때 텍스트 레코드 시작 주소 출력
                out << setfill('0') << setw(6) << right << strtoupper(ulltohex(addr_vec[i]));
                rec_flag = -1;
            }
            if (op_map.find(opcode) != op_map.end()) {  // optab에 존재하는 명령어일 때
                if (opcode == "LDCH" || opcode == "STCH") { // ldch, stch일 때 처리
                    string symbol = operand.substr(0, operand.size() - 2);  // ,x를 제외한 symbol
                    record += op_map[opcode] + ulltohex(symtab[symbol] + 32768);   // 인덱스 주소 지정 방식으로 16진수 8000을 더함
                }
                else {
                    if (opcode == "RSUB")   // opcode가 rsub일 때 처리
                        record += op_map[opcode] + "0000";
                    else {  // 그 외 명령어 처리
                        record += op_map[opcode] + ulltohex(symtab[operand]);
                    }    
                }
            }
            else if (opcode == "BYTE" || opcode == "WORD") {  // opcode가 byte나 word일 때
                if (opcode == "BYTE") {
                    if (operand[0] == 'x') {    // byte x'문자열', 문자열이 그대로 objcode가 됨.
                        operand = operand.substr(2, operand.size() - 3);
                        record += operand;
                        if (record.length() > 60) {    // 레코드 길이가 1E가 넘어가면 자른다.
                            record = record.substr(0, record.length() - operand.length());
                            record = record_size(record.length() / 2) + record;  // 바이트로 나타낸 레코드의 길이(16진수)와 16진수로 나타낸 목적 코드를 연결
                            out << strtoupper(record) << '\n';  // 텍스트 레코드를 출력
                            record = "";   // 레코드 초기화
                            newline = false;
                            // 텍스트 레코드가 짤리므로 다음 텍스트 레코드의 7열까지 출력
                            out << "T" << setfill('0') << setw(6) << right << strtoupper(ulltohex(addr_vec[i]));
                            rec_flag = -1;
                        }
                        record += operand; // 새로운 레코드에 붙임
                    }
                    else if (operand[0] == 'c') {   // byte c'문자열' 처리
                        operand = operand.substr(2, operand.size() - 3);
                        byte = strtoaskii(operand);  // 아스키 코드로 변환
                        record += byte;
                        if (record.length() > 60) {    // 레코드 길이가 1E가 넘어가면 
                            record = record.substr(0, record.length() - byte.length()); // 자른다.
                            record = record_size(record.length() / 2) + record;  // 바이트로 나타낸 레코드의 길이(16진수)와 16진수로 나타낸 목적 코드를 연결
                            out << strtoupper(record) << '\n';  // 텍스트 레코드를 출력
                            record = ""; rec_flag = 0;  // 레코드 초기화
                            newline = false;
                            // 텍스트 레코드가 짤리므로 다음 텍스트 레코드의 7열까지 출력
                            out << "T" << setfill('0') << setw(6) << right << strtoupper(ulltohex(addr_vec[i]));
                            rec_flag = -1;
                            record += byte; // 새로운 레코드에 붙임
                        }
                        newline = false;
                    }
                    else {  // 그 외의 operand는 오류
                        error(-1);
                    }
                } else if (opcode == "WORD") {  // opcode가 WORD일 때
                    record += inttostr(operand);    // operand를 0이 채워진 16진수 문자열로 바꿔서 record에 연결   
                    if (record.length() > 60) {    // 레코드 길이가 1E가 넘어가면 
                        record = record.substr(0, record.length() - 6); // 자른다.
                        record = record_size(record.length() / 2) + record;  // 바이트로 나타낸 레코드의 길이(16진수)와 16진수로 나타낸 목적 코드를 연결
                        out << strtoupper(record) << '\n';  // 텍스트 레코드를 출력
                        record = ""; rec_flag = 0;  // 레코드 초기화
                        newline = false;
                        // 텍스트 레코드가 짤리므로 다음 텍스트 레코드의 7열까지 출력
                        out << "T" << setfill('0') << setw(6) << right << strtoupper(ulltohex(addr_vec[i]));
                        rec_flag = -1;
                        record += inttostr(operand);    // 새로운 레코드에 붙임
                    }
                    newline = false;    // newline하지 않고 record에 연결
                }    
            }
            else if (opcode == "RESB" || opcode == "RESW") {    // opcode가 RESB, RESW일 때
                newline = true;
            }
            else {  // SIC 프로그램에 존재하는 opcode가 아닐 때
                out << setfill('0') << setw(6) << "0"; // 피연산자 주소를 0으로 설정
                error(-1);
            }
            if (newline || record.length() == 60) {   // 레코드 길이가 1E이거나 newline이 1일 때 출력
                if (record.length() != 0) { // 레코드의 길이가 동시에 0이 아닐 때만 출력
                    record = record_size(record.length() / 2) + record;  // 바이트로 나타낸 레코드의 길이(16진수)와 16진수로 나타낸 목적 코드를 연결
                    rec_flag = 0;
                    out << strtoupper(record) << '\n';  // 텍스트 레코드를 출력
                    record = "";    // 레코드 초기화
                    newline = false;
                }
            }
            i++;    // 파일 라인 인덱스 증가
        }
    }
}

int main()
{
    string file_name;
    cout << "파일 이름 입력: ";
    cin >> file_name;
    if (access(file_name.c_str(), F_OK) == 0) { // 입력 받은 파일이 존재할 때
        make_optab();   // optab 형성
        pass1(file_name);   // symtab 형성
        pass2(file_name);   // object code 출력
    }
    else {  // 입력 받은 파일이 존재하지 않을 경우 처리
        cout << "[Error] '" << file_name << "' File Not Found";
        return 0;
    }
    string obj_line, objcode;
    vector<string> objcode_vec;
    ifstream objfile("OBJFILE"); // "OBJFILE" 파일을 읽어온다.
    while (getline(objfile, obj_line)) {   // "OBJFLE" 파일을 한 줄씩 읽어온다.
        if (obj_line[0] == 'T') {
            obj_line = obj_line.substr(9, obj_line.size() - 9);  // 텍스트 레코드의 16진수 목적코드만 자름
            for (int i = 0; i < obj_line.size(); i++) {
                objcode += obj_line[i];
                if (objcode.size() == 6) {  // 6글자씩 16진수 목적코드를 잘라서 objcode_vec에 저장한다.
                    objcode_vec.push_back(objcode);
                    objcode.clear();
                }
            }
        } 
        else if (obj_line[0] == 'E')    // 엔드 레코드를 만나면 종료
            break;
        else if (obj_line[0] == 'H') {  // 헤더 레코드는 건너뜀
            continue;
        }
    }
    objfile.close();
    char input; // 실행 또는 종료를 입력 받음.
    int run;    // 한 번에 실행할 명령어 갯수
    int i = 0;  // srcfile 라인 추적
    typedef int Register;
    Register A = 0; // A 레지스터의 값
    cout << "실행: r, 종료: q\n";
    // cout << "한 번에 실행할 명령어 갯수 : ";
    // cin >> run; 
    while (true) {
        cin >> input;
        if (input == 'q' || i >= objcode_vec.size())    // q를 입력하거나 더 이상 실행할 라인이 없으면 종료
            break;
        string code = objcode_vec[i].substr(0, 2);  // 오브젝트 코드의 앞 두 자리: 연상 연산 코드
        string s_addr = objcode_vec[i].substr(2, 4);  // 오브젝트 코드의 뒤 네 자리: operand의 주소
        unsigned long long ull_addr = strtoull_addr(s_addr);
        stringstream ss(line_vec[i]), fm(file_map[ull_addr]);
        ss.str(line_vec[i]); fm.str(file_map[ull_addr]);
        // label, opcode, operand 공백 기준으로 자른다.
        string s_label, s_opcode, s_operand, f_label, f_opcode, f_operand;
        ss >> s_label >> s_opcode >> s_operand; // 명령어 출력을 위해 line_vec의 원소를 공백 기준으로 나눔
        fm >> f_label >> f_opcode >> f_operand; // 레지스터 계산을 위해 file_map의 value를 공백 기준으로 나눔
        if (s_operand.empty()) {    // label이 없는 경우를 처리
            s_operand = s_opcode; s_opcode = s_label; s_label = "-";
        }
        if (s_opcode.empty() && s_operand.empty()) {    // rsub와 같이 label과 operand가 없는 경우 처리
            s_opcode = s_label; s_label = "-"; s_operand = "-";
        }
        if (f_operand.empty()) {    // label이 없는 경우를 처리
            f_operand = f_opcode; f_opcode = f_label; f_label = "-";
        }
        if (f_opcode.empty() && f_operand.empty()) {    // rsub와 같이 label과 operand가 없는 경우 처리
            f_opcode = f_label; f_label = "-"; f_operand = "-";
        }

        if (s_opcode == "word" || s_opcode == "byte" || s_opcode == "resb" || s_opcode == "resw")
            break;  // 선언문이면 종료

        if (input == 'r') { // 'r'을 입력했을 때 처리
            cout << objcode_vec[i] << ' ' << s_opcode << ' ' << s_operand << ' ' << '\n';

            if (code == "00")   // lda 연산
                A = stoi(f_operand);
            else if (code == "18")  // add 연산
                A += stoi(f_operand);
            else if (code == "20")  // mul 연산
                A *= stoi(f_operand);
            else if (code == "1C")  // sub 연산
                A -= stoi(f_operand);
            cout << "REGISTER A: " << A << '\n';    // 레지스터에 담긴 값 출력
        }
        else
            cout << "Illegal instruction\n";
        i++;
    }
}