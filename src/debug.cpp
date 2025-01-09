#include <iostream>
#include <vector>
#include <map>
#include "swiftpaser.h"
#include "ccout.h"
#include "debug.h"

extern Swift swift;
void DebugExit()
{
    exit(0);
}
void LoadFile()
{
    ccout.font(f_color::RED) << "Input file url:";
    ccout.command(cc_command::FLUSH);
    std::string file;
    std::cin >> file;
    std::string url = file;
    ccout.font(f_color::RED)<<"Running\n";
    swift.RunPaser(url);
}

void ShowContainer()
{
    swift.ShowContainerByType(TYPE_UNKNOW);
}

void ShowStruct()
{
    swift.ShowContainerByType(TYPE_STRUCT);
    swift.ShowContainerByType(TYPE_ENUM);
    swift.ShowContainerByType(TYPE_CLASS);
}

void ShowLetVar()
{
    swift.ShowContainerByType(TYPE_LET);
    swift.ShowContainerByType(TYPE_SLET);
    swift.ShowContainerByType(TYPE_VAR);
    swift.ShowContainerByType(TYPE_SVAR);
}

void ShowExtension()
{
    swift.ShowContainerByType(TYPE_EXTENSION);
}

void ShowProtocol()
{
    swift.ShowContainerByType(TYPE_PROTOCOL);
}

void ShowFunction()
{
    swift.ShowContainerByType(TYPE_FUNC);
    swift.ShowContainerByType(TYPE_SFUNC);
}

void ShowOther()
{
    swift.ShowContainerByType(TYPE_LIAS);
    swift.ShowContainerByType(TYPE_SUBSCRIPT);
}

void LoadJson()
{
    ccout.font(f_color::RED) << "Input file url:";
    ccout.command(cc_command::FLUSH);
    std::string file;
    std::cin >> file;
    swift + file;
}

void SaveJson()
{   
    -swift;  
}

void ShowKeyInfo()
{
    ccout.font(f_color::RED) << "Input keyword:";
    ccout.command(cc_command::FLUSH);
    std::string key;
    std::cin >> key;

    ccout.font(f_color::RED) << "Input level:";
    ccout.command(cc_command::FLUSH);
    int level;
    std::cin >> level;
    ccout.command(cc_command::CLS);
    swift.Print(key,level);
}

typedef void (*CmdFunc)(); 
std::map<std::string, CmdFunc> CmdMap = {
    {"0", DebugExit},
    {"1", LoadFile},
    {"2", ShowContainer},
    {"3", ShowStruct},
    {"4", ShowLetVar},
    {"5", ShowExtension},
    {"6", ShowProtocol},
    {"7", ShowFunction},
    {"8", ShowOther},
    {"9", LoadJson},
    {"10", SaveJson},
    {"11", ShowKeyInfo},
};

CmdFunc ShowCMD()
{
    ccout.font(f_color::GREEN) << "0" << ccout.font(f_color::NONE)<<": exit\n";
    ccout.font(f_color::GREEN) << "1" << ccout.font(f_color::NONE)<<": load swift file\n";
    ccout.font(f_color::GREEN) << "2" << ccout.font(f_color::NONE)<<": show container\n";
    ccout.font(f_color::GREEN) << "3" << ccout.font(f_color::NONE)<<": show struct\n";
    ccout.font(f_color::GREEN) << "4" << ccout.font(f_color::NONE)<<": show letvar\n";
    ccout.font(f_color::GREEN) << "5" << ccout.font(f_color::NONE)<<": show extension\n";
    ccout.font(f_color::GREEN) << "6" << ccout.font(f_color::NONE)<<": show protocol\n";
    ccout.font(f_color::GREEN) << "7" << ccout.font(f_color::NONE)<<": show function\n";
    ccout.font(f_color::GREEN) << "8" << ccout.font(f_color::NONE)<<": show other\n";

    ccout.font(f_color::GREEN) << "9" << ccout.font(f_color::NONE)<<": load json file\n";
    ccout.font(f_color::GREEN) << "10" << ccout.font(f_color::NONE)<<": save json file\n";
    ccout.font(f_color::GREEN) << "11" << ccout.font(f_color::NONE)<<": show key info\n";



WAIT_INPUT:
    ccout.font(f_color::RED) << "Input cmd number:";
    ccout.command(cc_command::FLUSH);
    std::string number;
    std::cin >> number;

    auto it = CmdMap.find(number);
    if(it == CmdMap.end())
    {
        ccout.font(f_color::RED) << "\nInput error\n";
        goto WAIT_INPUT;
    }
    ccout.command(cc_command::CLS);
    return CmdMap[number];
}

void Debug(std::string number)
{
    auto it = CmdMap.find(number);
    if(it != CmdMap.end())
    {
        ccout.command(cc_command::CLS);
        CmdMap[number]();
    }
    while(1)
    {
        ShowCMD()(); 
    }
}