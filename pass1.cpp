#include<iostream>
#include<fstream>
#include<vector>
#include <iomanip>
#include <string>
#include<algorithm>
#include<unordered_map>
using namespace std;

unordered_map<string, string>OPTAB={
    {"ADD","18"},    // Add
    {"AND","40"},    // And
    {"COMP","28"},   // Compare
    {"DIV","24"},    // Divide
    {"J","3C"},      // Jump
    {"JEQ","30"},    // Jump if equal to zero
    {"JGT","34"},    // Jump if greater than
    {"JLT","38"},    // Jump if less than
    {"JSUB","48"},   // Jump to subroutine
    {"LDA","00"},    // Load accumulator
    {"LDB","68"},    // Load B register
    {"LDCH","50"},   // Load character
    {"LDF","70"},    // Load F register
    {"LDL","08"},    // Load L register
    {"LDS","6C"},    // Load S register
    {"LDT","74"},    // Load T register
    {"LDX","04"},    // Load X register
    {"MUL","20"},    // Multiply
    {"OR","44"},     // Or
    {"RD","D8"},     // Read
    {"RSUB","4C"},   // Return from subroutine
    {"STA","0C"},    // Store accumulator
    {"STB","78"},    // Store B register
    {"STCH","54"},   // Store character
    {"STF","80"},    // Store F register
    {"STL","14"},    // Store L register
    {"STS","7C"},    // Store S register
    {"STSW","E8"},   // Store status word
    {"STT","84"},    // Store T register
    {"STX","10"},    // Store X register
    {"SUB","1C"},    // Subtract
    {"TD","E0"},     // Test device
    {"TIX","2C"},    // Test and increment index
    {"WD","DC"}      // Write
};

unordered_map<string,int>format2=
{
    {"SVC", 1},{"CLEAR", 2},{"ADDR", 2},{"COMPR", 2},{"SUBR", 2},{"MULR", 2},{"DIVR", 2},{"RMO", 2},{"SHIFTL", 2},{"SHIFTR", 2},{"TIXR", 2},{"AND", 2},{"OR", 2},{"NOT", 2}
};

void parseLine(string &line,string&label,string&instruction,string&operand)
{
    int l=line.length();
    string temp="";
    bool foundLabel=false;
    int i=0;
    while(line[i]!=' '&&i<l)
    {
        temp+=line[i];
        i++;
    }
    label=temp;
    temp="";
    while(line[i]==' '&&i<l)
    i++;
    while(line[i]!=' '&&i<l)
    {
        temp+=line[i];
        i++;
    }
    instruction=temp;
    temp="";
    while(line[i]==' '&&i<l)
    i++;
    while(line[i]!=' '&&i<l)
    {
        operand+=line[i];
        i++;
    }
}

vector<pair<string,int>>pass1(ifstream &inputFile,int &e,int&start_add,int&loc_ctr)
{
    vector<pair<string,int>>symtab;
    string line;
    bool started=false;
    while(getline(inputFile,line))
    {
        string label="",instruction="",operand="";
        parseLine(line,label,instruction,operand);
        operand.erase(operand.find_last_not_of(" \n\r\t")+1);
        instruction.erase(instruction.find_last_not_of(" \n\r\t")+1);
        if(instruction=="END")
        break;
        if(instruction=="START")
        {
            start_add=stoi(operand,nullptr,16);
            loc_ctr=start_add;
            started=true;
            continue;
        }
        if(!started)
        {
            e=1;
            cout<<"No Starting of the Program\n";
            return symtab;
        }
        if(instruction=="BASE")
        continue;
        if(!label.empty())
        {
            if(find_if(symtab.begin(), symtab.end(),
                      [&label](const pair<string, int>& p) {
                          return p.first == label; // Condition to match
                      })!=symtab.end())
            {
                e=1;
                cout<<"Duplicate Values\n";
                return symtab;
            }
            else
            symtab.push_back({label,loc_ctr});
        }
        if(instruction=="RESW")
        {
            loc_ctr+=3*stoi(operand);
        }
        else if(instruction=="RESB")
        {
            loc_ctr+=stoi(operand);
        }
        else if(instruction=="WORD")
        {
            loc_ctr+=3;
        }
        else if(instruction=="BYTE")
        {
            if(operand[0]=='C')
            loc_ctr+=operand.length()-3;
            else if(operand[0]=='X')
            loc_ctr+=(operand.length()-3)/2;
        }
        else if(format2.find(instruction)!=format2.end())
        {
            loc_ctr+=format2[instruction];
        }
        else if(instruction[0]=='+')
        {
            loc_ctr+=4;
        }
        else if(instruction[0]=='@')
        {
            loc_ctr+=3;
        }
        else if(OPTAB.find(instruction)!=OPTAB.end())
        {
            loc_ctr+=3;
        }
        else
        {
            e=1;
            cout<<"Invalid Instruction";
            return symtab;
        }
    }
    return symtab;
}

int main()
{
    ifstream inputFile("Input.txt");
    if(!inputFile)
    {
        cout<<"Error opening in File";
        return 1;
    }
    int e=0,start_add=0,length=0;
    vector<pair<string,int>>SYMTAB=pass1(inputFile,e,start_add,length);
    if(e)
    {
        return 1;
    }
    ofstream outFile("Symtab.txt");
    for(auto x:SYMTAB)
    {
        outFile<<x.first<<"\t"<<hex<<uppercase<<setw(4)<<setfill('0')<<x.second<<endl;
    }
    inputFile.close();
    return 0;
    
}