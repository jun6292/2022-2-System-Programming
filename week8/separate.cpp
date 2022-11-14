#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

int main()
{
    string line;    // 파일 내용을 한줄씩 저장할 string 변수
    ifstream file("SRCFILE"); // SRCFILE 파일을 읽어온다.
    vector<string> label_vec, opcode_vec, operand_vec;  // label, opcode, operand로 분리하여 저장할 자료구조

    while (getline(file, line)) {   // 파일을 한 줄씩 읽어온다.
        stringstream ss(line);
        ss.str(line);

        string label, opcode, operand;
        ss >> label >> opcode >> operand;   // label, opcode, operand 공백 기준으로 자른다.
        if (operand.empty()) {  // label이 없는 경우를 처리
            operand = opcode;
            opcode = label;
            label = "-    ";
        }
        // 각각의 벡터에 label, opcode, operand를 push_back
        label_vec.push_back(label);
        opcode_vec.push_back(opcode);
        operand_vec.push_back(operand);
    }
    // 분리하여 저장한 결과 출력
    for (int i = 0; i < label_vec.size(); i++) {
        cout << "Label: " << label_vec[i] << ", ";
        cout << "Opcode: " << opcode_vec[i] << ", ";
        cout << "Operand: " << operand_vec[i] << "\n";
    }
}