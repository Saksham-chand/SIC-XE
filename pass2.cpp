#include<iostream>
#include<vector>
#include <string>
#include<fstream>
#include <iomanip>
#include<unordered_map>
#include<algorithm>

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

unordered_map<string,string>format2=
{
        {"SVC", "B0"},
        {"CLEAR", "B4"},
        {"ADDR", "90"},
        {"COMPR", "A0"},
        {"SUBR", "94"},
        {"MULR", "98"},
        {"DIVR", "9C"},
        {"RMO", "AC"},
        {"SHIFTL", "A4"},
        {"SHIFTR", "A8"},
        {"TIXR", "B8"},
        {"AND", "40"},
        {"OR", "44"},
        {"NOT", "50"}
};

int bp=0,pc=0;
unordered_map<string,string>Registers={{"A","0"},{"B","3"},{"S","4"},{"T","5"},{"F","6"},{"X","1"}};

void create_symtab(vector<pair<string,int>>&symtab)
{
    ifstream inFile("Symtab.txt");
    string label,add;
    while(inFile>>label>>add)
    {
        symtab.push_back({label,stoi(add,nullptr,16)});
    }
    inFile.close();
}

string intToHex(int value,int width) {
    stringstream stream;
    stream<<setw(width)<<setfill('0')<<hex<<uppercase<<value;
    return stream.str();
}

bool is12BitDisplacement(int displacement) {
    return (displacement>=0&&displacement<=4095);
}

string constructInstructionCode(string& opcodeHex,bool n,bool i,bool x,bool b,bool p,bool e,int displacement) {
    int opcode=stoi(opcodeHex,nullptr,16);
    int ni=(n<<1)|i;
    opcode=(opcode&0xFC)|ni;
    int xbpe=(x<<3)|(b<<2)|(p<<1)|e;
    //cout<<hex<<opcodeHex<<' '<<pc<<' '<<bp<<' '<<displacement<<endl;
    if(!e) 
    {
        if(p||b)
        {
            if(p)
            {
                int dp=displacement-pc;
                if((dp)>0&&is12BitDisplacement(dp))
                {
                    int instruction=(opcode<<16)|(xbpe<<12)|((dp)&0xFFF);
                    return intToHex(instruction,6);
                }
                int temp=abs(static_cast<int>(displacement)-static_cast<int>(pc));
                if(!is12BitDisplacement(temp))
                goto x;
                dp=~dp;
                dp=0xFFF-dp;
                dp=0xFFF&dp;
                dp=static_cast<int>(dp);
                if(is12BitDisplacement(dp))
                {
                    int instruction=(opcode<<16)|(xbpe<<12)|(dp&0xFFF);
                    return intToHex(instruction,6);
                }
            }
            x:p=false;
            b=true;
            xbpe=(x<<3)|(b<<2)|(p<<1)|e;
            if(b)
            {
                int db=displacement-bp;
                if((db)>0&&is12BitDisplacement(db))
                {
                    int instruction=(opcode<<16)|(xbpe<<12)|(db&0xFFF);
                    return intToHex(instruction,6);
                }
                db=~db;
                db=0xFFF-db;
                db=0xFFF&db;
                db=static_cast<int>(db);
                if(is12BitDisplacement(db))
                {
                    int instruction=(opcode<<16)|(xbpe<<12)|(db&0xFFF);
                    return intToHex(instruction,6);
                }
            }
        }
        int instruction=(opcode<<16)|(xbpe<<12)|(displacement&0xFFF);
        return intToHex(instruction,6);
    } 
    else 
    {
        int instruction=(opcode<<24)|(xbpe<<20)|(displacement&0xFFFFF);
        return intToHex(instruction,8);
    }
    return "";
}

int findAddress(vector<pair<string, int>> symtab, string operand) {
    operand.erase(operand.find_last_not_of(" \n\r\t")+1);
    auto it=find_if(symtab.begin(),symtab.end(),[&operand](const pair<string,int>& x) 
    {
        return x.first==operand;
    });
    if(it!=symtab.end()) {
        return it->second;
    }
    return -1;
}

void parseLine(string &line,string &label,string &instruction,string &operand)
{
    int l=line.length(),i=0;
    while(i<l&&line[i]!=' ')
    {
        label+=line[i];
        i++;
    }
    while(i<l&&line[i]==' ')
    i++;
    while(i<l&&line[i]!=' ')
    {
        instruction+=line[i];
        i++;
    }
    while(i<l&&line[i]==' ')
    i++;
    while(i<l)
    {
        operand+=line[i];
        i++;
    }
}

void pass2(ifstream &inputFile,vector<pair<string,int>> &symtab,int &e)
{
    string line;
    int loc_ctr=0;
    bool start=false;
    ofstream outputFile("Output.obj");
    if (!outputFile) {
        cerr<<"Error opening output file\n"<<endl;
        return ;
    }
    while(getline(inputFile,line))
    {
        string label,instruction,operand;
        parseLine(line,label,instruction,operand);
        operand.erase(operand.find_last_not_of(" \n\r\t")+1);
        instruction.erase(instruction.find_last_not_of(" \n\r\t")+1);
        if(instruction=="RSUB")
        {
            outputFile<<"4F0000"<<endl;
            pc+=0x3;
            continue;
        }
        if(instruction=="START")
        {
            loc_ctr=stoi(operand);
            start=true;
            pc=stoi(operand);
            continue;
        }
        if(instruction=="BASE")
        continue;
        if(!start)
        {
            e=1;
            return;
        }
        if(instruction=="LDB")
        {
            if(operand[0]=='#')
            operand=operand.substr(1);
            bp=(findAddress(symtab,operand));
            pc+=0x3;
            string opcodeHex=OPTAB[instruction];
            bool n,i,x=false,b,p,e;
            n=false;
            i=true;
            e=false;
            p=true;
            b=false;
            int l=operand.length();
            if(operand.length()>2&&operand[l-1]=='X'&&operand[l-2]==',')
            {
                x=true;
                operand=operand.substr(0,operand.size()-2);
            }
            int opcode=stoi(opcodeHex,nullptr,16);
            int ni=(n<<1)|i;
            opcode=(opcode&0xFC)|ni;
            int xbpe=(x<<3)|(b<<2)|(p<<1)|e;
            int instruction=(opcode<<16)|(xbpe<<12)|((bp-pc)&0xFFF);
            outputFile<<intToHex(instruction,6)<<setw(4)<<setfill('0')<<hex<<uppercase<<endl;
            continue;
        }
        if(instruction=="END")
        break;
        if(format2.find(instruction)!=format2.end())    //format 2
        {
            string op=format2[instruction];
            pc+=0x2;
            if(operand.length()==1)
            {
                string r=Registers[operand];
                string a=Registers["A"];
                outputFile<<setw(2)<<op<<r<<a<<endl;
            }
            else
            {
                string r(1,operand[0]);
                r=Registers[r];
                string a(1,operand[2]);
                a=Registers[a];
                outputFile<<setw(4)<<op<<r<<a<<endl;
            }
        }
        else if(OPTAB.find(instruction)!=OPTAB.end())   //format 3
        {
            pc+=0x3;
            bool n,i,x=false,b,p,e;
            int disp;
            string opcode=OPTAB[instruction];
            int l=operand.length(),add;
            if(operand.length()>2&&operand[l-1]=='X'&&operand[l-2]==',')
            {
                x=true;
                operand=operand.substr(0,operand.size()-2);
            }
            if(operand[0]=='#')
            {
                operand=operand.substr(1);
                n=false;
                i=true;
                e=false;
                p=false;
                b=false;
                if(isdigit(operand[0]))
                disp=stoi(operand);
                else
                disp=findAddress(symtab,operand);
                outputFile<<constructInstructionCode(opcode,n,i,x,b,p,e,disp)<<setw(4)<<setfill('0')<<hex<<uppercase<<endl;
            }
            else if(operand[0]=='@')
            {
                operand=operand.substr(1);
                n=true;
                i=false;
                e=false;
                p=true;
                b=false;
                if(isdigit(operand[0]))
                disp=stoi(operand);
                else
                disp=findAddress(symtab,operand);
                string code=constructInstructionCode(opcode,n,i,x,b,p,e,disp);
                outputFile<<code<<setw(4)<<setfill('0')<<hex<<uppercase<<endl;
            }
            else
            {
                n=true;
                i=true;
                e=false;
                p=true;
                b=false;
                disp=findAddress(symtab,operand);
                string code=constructInstructionCode(opcode,n,i,x,b,p,e,disp);
                outputFile<<code<<setw(4)<<setfill('0')<<hex<<uppercase<<endl;
            }
        }
        else if(instruction[0]=='+')    //format 4
        {
            pc+=0x4;
            bool n,i,x=false,b,p,e;
            instruction=instruction.substr(1);
            string opcode=OPTAB[instruction];
            int l=operand.length(),add;
            if(operand.length()>2&&operand[l-1]=='X'&&operand[l-2]==',')
            {
                x=true;
                operand=operand.substr(0,operand.size()-2);
            }
            if(!isdigit(operand[1]))
            add=findAddress(symtab,operand);
            else
            add=stoi(operand.substr(1,l));
            if(add==-1)
            {
                e=1;
                cout<<operand<<endl;
                return; 
            }
            if(operand[0]=='#')
            {
                operand=operand.substr(1);
                n=false;
                i=true;
                b=false;
                p=false;
                e=true;
            }
            else if(operand[0]=='@')
            {
                operand=operand.substr(1);
                n=true;
                i=false;
                b=false;
                p=false;
                e=true;
            }
            else
            {
                n=true;
                i=true;
                b=false;
                p=false;
                e=true;
            }
            outputFile<<constructInstructionCode(opcode,n,i,x,b,p,e,add)<<setw(4)<<setfill('0')<<hex<<uppercase<<endl;
        }
        else if(instruction=="WORD")
        {
            outputFile<<setw(6)<<setfill('0')<<hex<<uppercase<<stoi(operand)<<endl;
            pc+=0x3;
        }
        else if(instruction=="BYTE")
        {
            if(operand[0]=='X')
            {
                outputFile<<operand.substr(2,operand.size()-3)<<endl;
                string temp=operand;
                temp=temp.substr(2,temp.size()-3);
                int l=temp.length();
                pc+=(l/2);
            }
            else if(operand[0]=='C')
            {
                for (int j=2;j<operand.size()-1;j++) 
                {
                    char currentChar=operand[j];
                    outputFile<<setw(2)<<setfill('0')<<hex<<uppercase<<(int)(unsigned char)currentChar<<endl;
                    pc+=0x1;
                }
            }
        }
        else if(instruction=="RESW")
        {
            outputFile<<"RESW_"<<operand<<endl;
            pc+=(3*stoi(operand));
        }
        else if(instruction=="RESB")
        {
            outputFile<<"RESB_"<<operand<<endl;
            pc+=stoi(operand);
        }
        else
        {
            e=1;
            return;
        }
    }
}

int main()
{
    ifstream inputFile("Input.txt");
    int e=0;
    if(!inputFile)
    {
        cout<<"Error in opening File\n";
        return 1;
    }
    vector<pair<string,int>>symtab;
    create_symtab(symtab);
    pass2(inputFile,symtab,e);
    if(e==1)
    {
        cout<<"ERROR\n";
        return 1;
    }
    inputFile.close();
    return 0;
}