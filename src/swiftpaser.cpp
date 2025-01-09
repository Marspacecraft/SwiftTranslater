#include <algorithm>
#include <sstream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <errno.h>
#include "ccout.h"
#include "swiftpaser.h"

#define REGX_ACCESS R"(\s*(open|public|internal|fileprivate|private|internal)?\s*(mutating)?\s*)"
#define REGX_CLASS_HEAD REGX_ACCESS
#define REGX_CLASS_TAIL R"(\s+([\w.]+)(<[\w\s,]+>)?\s*(:([\w\s,.:=]*))?\s*(where\s*([\w\s:.,=<>]+))?\s*)"

#define REGX_VARIABLE_HEAD R"((open|public|internal|fileprivate|private|internal)?\s*(static)?\s*)"
#define REGX_VARIABLE_TAIL R"(\s+([\w\$`]+)\s*(:([\w\s,<>.]*))?\s*)"


/***********************************
 * static func
 * *********************************/
 static colorcout& TabPrint(int level,f_color color)
 {
    for(int i=1;i<level;i++)
        ccout<<"\t";
    return ccout.font(color);
 }

//调用脚本进行google翻译
static void exec(const std::string cmd,std::string& result) 
{
    std::array<char, 128> buffer;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) 
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
    {
        result += buffer.data();
    }
}

static std::string& replaceAllquotes(std::string& str) 
{
    size_t start_pos = 0;
    while ((start_pos = str.find("\"", start_pos)) != std::string::npos) 
    {
        str.replace(start_pos, 1, "\\\"");
        start_pos += 2;
    }
    return str;
}

static void PrintByLevel(f_color color,std::string& text,int level)
{
    std::istringstream stream(text);
    std::string line;
    while(std::getline(stream, line))
    {
        TabPrint(level,color);
        ccout.font(color)<<line<<"\n";
    }
}

 inline void PrintSpace()
 {
    ccout<<" ";
 }
 inline void PrintEnter()
 {
    ccout<<"\n";
 }

// 匹配()层数
static std::string MatchParenthesis(const std::string& text, const std::string& keyword) 
{
   size_t keywordPos = text.find(keyword);
    if (keywordPos == std::string::npos) {
        return "";
    }

    size_t start = text.find('(', keywordPos);
    if (start == std::string::npos) {
        return "";
    }

    std::stack<char> parenthesesStack;
    size_t end = start;

    for (size_t i = start; i < text.length(); ++i) {
        if (text[i] == '(') {
            parenthesesStack.push('(');
        } else if (text[i] == ')') {
            parenthesesStack.pop();
            if (parenthesesStack.empty()) {
                end = i;
                break;
            }
        }
    }

    if (end > start) {
        return text.substr(start + 1, end - start - 1);
    }
    return "";
}

static void DelSubString(std::string& text, const std::string& keyword)
{
    if("" == keyword)
        return;

    size_t pos = text.find(keyword);

    if (pos != std::string::npos && (pos + keyword.length()) <= text.length()) 
    {
        text.erase(0, pos + keyword.length());
    }
}
// 参数比较复杂，正则表达式无法匹配
// 同时处理返回值和约束
static void MatchFuncParamter(const std::string& name,SwiftBase& base,const std::string& line)
{
    std::string text = line;
    std::string param = MatchParenthesis(text,name);

    base.SetExtend2(param); 

    DelSubString(text,param);
    if("" == text)
        return;
        
    std::regex pattern(R"(\s*->([\w\s:.\[\],\?=<>]+)\s*\{?)");
    std::smatch match;

    if (std::regex_search(text, match, pattern))
        base.SetExtend1(match[1]);//return

    DelSubString(text,match[1]);
    if("" == text)
        return;
     pattern = std::string(R"(\s*(where\s*([\w\s:.\[\]=\?,<>]+))\s*\{?)");

    if (std::regex_search(text, match, pattern))   
        base.SetExtend3(match[2]);//where

    return;
}

/***********************************
 * LinesBuffer
 * *********************************/
void LinesBuffer::GetBlock(std::string& buffer,const char blockleft,const char blockright,int hadleftcount)
{
    int matchcount = hadleftcount;
    int matchpos = 0;    

    for (size_t i = 0; i < _content.size(); ++i) 
    {
        if (_content[i] == blockleft) 
        {
            if(0 == matchcount)
                matchpos=i+1;
            ++matchcount;
        } 
        else if (_content[i] == blockright) 
        {
            if(matchcount)
                --matchcount;
            if (!matchcount) 
            {  
                buffer = _content.substr(matchpos, i - matchpos);       
                _content.erase(0,i);
                PopLine();
                break;            
            }
        }
    }
}

/***********************************
 * SwiftBase
 * *********************************/

bool SwiftBase::PrintType() const
{
    f_color color = f_color::PURPLE;
    switch(_type)
    {
        case TYPE_ANNOTATION:
            ccout.font(color)<< "annotation";
        return true;
        case TYPE_AVAILABLE:
            ccout.font(color)<< "available";
        return true;
        case TYPE_STRUCT:
            ccout.font(color)<< "struct";
        return true;
        case TYPE_CLASS:
            ccout.font(color)<< "class";
        return true;
        case TYPE_PROTOCOL:
            ccout.font(color)<< "protocol";
        return true;
        case TYPE_ASSOCIATE:
            ccout.font(color)<< "associate";
        return true;
        case TYPE_EXTENSION:
            ccout.font(color)<< "extension";
        return true;
        case TYPE_ENUM:
            ccout.font(color)<< "enum";
        return true;
        case TYPE_ENUMVALUE:
            ccout.font(color)<< "enum case";
        return true;
        case TYPE_SVAR:
            ccout.font(color)<< "static_var";
        return true;
        case TYPE_VAR:
            ccout.font(color)<< "var";
        return true;
        case TYPE_SLET:
            ccout.font(color)<< "static_let";
        return true;
        case TYPE_LET:
            ccout.font(color)<< "let";
        return true;
        case TYPE_SUBSCRIPT:
            ccout.font(color)<< "subscript";
        return true;
        case TYPE_SFUNC:
            ccout.font(color)<< "static_func";
        return true;
        case TYPE_FUNC:
            ccout.font(color)<< "func";
        return true;
        case TYPE_LIAS:
            ccout.font(color)<< "lias";
        return true;
        case TYPE_INIT: 
            ccout.font(color)<< "init_func";
        return true;
        case TYPE_IMPORT: 
            ccout.font(color)<< "import";
        return true;
        case TYPE_NONE: 
            ccout.font(color)<< "none";
        return true;
        default:
        return false;
    }
    return false;
}

bool SwiftBase::PrintName() const
{
    f_color color = f_color::PURPLE;

    if(_name.length())
    {
        ccout.font(color) << _name;
        return true;
    }

    return false;
}

bool SwiftBase::PrintInherit() const
{
    f_color color = f_color::GREEN;

    if(0 == _inherit.length())
        return false;

    ccout.font(color) << _inherit;
    return true;
}

bool SwiftBase::PrintAnnotation(int level) const
{
    f_color color = f_color::CYAN;

    if(0 == _annotation.size())
        return false;

    for(int i=0;i<_annotation.size();++i)
        TabPrint(level,color) << _annotation[i] << "\n";
    
    return true;
}

bool SwiftBase::PrintAccess() const
{
    f_color color = f_color::PURPLE;
    if(0 == _access.length())
        return false;
    ccout.font(color) << _access;
    return true;
}

bool SwiftBase::PrintExtend1() const
{
    f_color color = f_color::BLUE;
    if(0 == _extend1.length())
        return false;
    ccout.font(color) << _extend1;
    return true;
}

bool SwiftBase::PrintExtend2() const
{
    f_color color = f_color::BLUE;
    if(0 == _extend2.length())
        return false;
    ccout.font(color) << _extend2;
    return true;
}

bool SwiftBase::PrintExtend3() const
{
    f_color color = f_color::BLUE;
    if(0 == _extend3.length())
        return false;
    ccout.font(color) << _extend3;
    return true;
}

bool SwiftBase::PrintContext() const
{
    f_color color = f_color::YELLOW;
    if(0 == _context.length())
        return false;
    ccout.font(color) << _context;
    return true;
}

bool SwiftBase::PrintAvailable() const
{
    f_color color = f_color::CYAN;
    if(0 == _available.length())
        return false;
    ccout.font(color) << _available;
    return true;
}

bool SwiftBase::PrintChinese(int level)
{
    if(0 == _annotation.size())
        return false;
    if(0 == _annotation_cn.length())
    {
        std::string cmd(SWIFT_JSON_DIRECTORY);
        cmd.append("GoogleTranslate.py \"");
        std::string param;
        for(int i=0;i<_annotation.size();++i)
            param.append(_annotation[i]).append("\n");
        // "号可能会打断脚本参数
        cmd.append(replaceAllquotes(param)).append("\"");
        exec(cmd,_annotation_cn);
        SetChineseFg();
    }

    f_color color = f_color::CYAN;
    if(0 == _annotation_cn.length())
        return false;
    PrintByLevel(color,_annotation_cn,level);
    return true;
}

void SwiftBase::Print() 
 {
    std::cout<<"Annotation:\n";
    PrintAnnotation();
    
    std::cout<<"Available:\n";
    PrintAvailable();
    std::cout<<"\n";

    std::cout<<"Context:\n";
    PrintContext();
    std::cout<<"\n";

    std::cout<<"Name:\n";
    PrintAccess();
    std::cout<<" ";
    PrintType();
    std::cout<<" ";
    PrintName();
    std::cout<<":";
    PrintInherit();
    std::cout<<std::endl;

    std::cout<<"Extend:\nE1 ";
    PrintExtend1();
    std::cout<<"\tE2 ";
    PrintExtend2();
    std::cout<<"\tE3 ";
    PrintExtend3();
    std::cout<<std::endl;
 }

 // 正则表达式匹配类型
bool SwiftBase::TypeMatch(SwiftPaserType type,const std::string& line)
{
    std::smatch match;
    std::regex pattern; 
    try
    {
        switch(type)
        {
            case TYPE_ANNOTATION:// 注释
                pattern = std::string(R"(///*(.*))");
                if (std::regex_search(line, match, pattern))
                { 
                    AddAnnotation(match[1]);
                    SetName(std::string("annotation"));
                    SetType(type);
                    return true;
                }
                
            break;
            case TYPE_AVAILABLE:// 版本
                pattern = std::string(R"(@available\s*\((.*)\))");
                if (std::regex_search(line, match, pattern)) 
                {
                    SetAvailabe(match[1]);
                    SetName(std::string("available"));
                    SetType(type);
                    return true;
                }
            break;
            case TYPE_STRUCT:// 结构体
                pattern = std::string(REGX_CLASS_HEAD)
                            +std::string(R"(struct)")
                            +std::string(REGX_CLASS_TAIL);
                goto CLASS_SET;
            case TYPE_CLASS:// class 
                pattern = std::string(REGX_CLASS_HEAD)
                            +std::string(R"(class)")
                            +std::string(REGX_CLASS_TAIL);
                goto CLASS_SET;
            break;
            case TYPE_PROTOCOL:// 协议
                pattern = std::string(REGX_CLASS_HEAD)
                            +std::string(R"(protocol)")
                            +std::string(REGX_CLASS_TAIL);
                goto CLASS_SET;
            break;
            case TYPE_ASSOCIATE:// 协议类型声明
                pattern = std::string(R"(\s*associatedtype\s+(\w*\s*:?\s*\w*))");
                if (std::regex_search(line, match, pattern))
                { 
                    SetName(match[1]);
                    SetType(type);
                    return true;
                }
            break;
            case TYPE_EXTENSION:// 扩展 
                pattern = std::string(REGX_CLASS_HEAD)
                            +std::string(R"(extension)")
                            +std::string(REGX_CLASS_TAIL);
    CLASS_SET:
                if (std::regex_search(line, match, pattern)) 
                {
                    if(match[3].length())
                    {
                        std::string name(match[3]);
                        name.append(match[4]);
                        SetName(name);
                        SetAccess(match[1]);
                        SetInherit(match[6]);
                        SetExtend3(match[8]);
                        SetType(type);
                        return true;
                    }
                }
            break;
            case TYPE_ENUM:// enum
                pattern = std::string(REGX_CLASS_HEAD)
                            +std::string(R"(enum)")
                            +std::string(REGX_CLASS_TAIL);
                goto CLASS_SET;
            break;
            case TYPE_ENUMVALUE:// case
                pattern = std::string(R"(\s*case\s+([\w`\(\)]+)\s*)");
                if (std::regex_search(line, match, pattern))
                { 
                    SetName(match[1]);
                    SetType(type);
                    return true;
                }
            break;
            case TYPE_SVAR:
            case TYPE_VAR:// var
                pattern = std::string(REGX_VARIABLE_HEAD)
                            +std::string(R"(var)")
                            +std::string(REGX_VARIABLE_TAIL);
                goto VARIABLE_SET;
            break;
            case TYPE_SLET:// 静态常量属性
            case TYPE_LET:// let
                pattern = std::string(REGX_VARIABLE_HEAD)
                            +std::string(R"(let)")
                            +std::string(REGX_VARIABLE_TAIL);
    VARIABLE_SET:
                if (std::regex_search(line, match, pattern)) 
                {
                    if(match[3].length())
                    {
                        SetName(match[3]);
                        SetAccess(match[1]);
                        SetInherit(match[5]);
                        if(match[2].length())
                        {
                            if(TYPE_SVAR==type||TYPE_VAR==type)
                                SetType(TYPE_SVAR);
                            else
                                SetType(TYPE_SLET);
                        }
                        else
                        {
                            if(TYPE_SVAR==type||TYPE_VAR==type)
                                SetType(TYPE_VAR);
                            else
                                SetType(TYPE_LET);
                        }
                        
                        return true;
                    }
                }
            break;
            // case TYPE_SUBSCRIPT:// 下标
            //     pattern = std::string(REGX_ACCESS)
            //                 +std::string(R"((static)?\s*subscript\s*(<[\w,\s\?]+>)?\s*)");// name
            //     if (std::regex_search(line, match, pattern)) 
            //     {
            //         std::string name("subscript");
            //         name.append(match[4]);
            //         SetAccess(match[1]);
            //         MatchFuncParamter(name,*this,line);
            //         SetType(type);
            //         return true;
            //     }
            // break;
            // 正则表达式过于复杂会抛异常
            case TYPE_SUBSCRIPT:// 下标
            {
                pattern = std::string(REGX_ACCESS);
                std::regex_search(line, match, pattern);
                std::string access = match[1];
                pattern =std::string(R"((static)?\s*subscript\s*(<[\w,\s\?]+>)?\s*)");// name
                if (std::regex_search(line, match, pattern)) 
                {
                    std::string name("subscript");
                    name.append(match[2]);
                    SetAccess(access);
                    MatchFuncParamter(name,*this,line);
                    SetType(type);
                    return true;
                }
            }
            break;
            // case TYPE_INIT:  // 构造函数
            //     pattern = std::string(REGX_ACCESS)
            //                 +std::string(R"(init(<[\w,:]+>|\?)?\s*)");// name
            //     if (std::regex_search(line, match, pattern)) 
            //     {
            //         std::string name("init");
            //         name.append(match[3]);
            //         if(name.length())
            //         {
            //             MatchFuncParamter(name,*this,line);
            //             SetName(name);
            //             SetAccess(match[1]);
            //             SetType(type);
            //             return true;
                        
            //         }
            //     }
            // break;
            case TYPE_INIT:  // 构造函数
            {
                pattern = std::string(REGX_ACCESS);
                std::regex_search(line, match, pattern);
                std::string access = match[1];
                pattern = std::string(R"(init(<[\w,:]+>|\?)?\s*)");// name
                if (std::regex_search(line, match, pattern)) 
                {
                    std::string name("init");
                    name.append(match[1]);
                    if(name.length())
                    {
                        MatchFuncParamter(name,*this,line);
                        SetName(name);
                        SetAccess(access);
                        SetType(type);
                        return true;
                        
                    }
                }
            }
            break;
            // case TYPE_SFUNC:// 类函数
            // case TYPE_FUNC:// func
            //     pattern = std::string(REGX_ACCESS)
            //                 +std::string(R"((static)?\s*func\s+([\+\-\*/=!%&\|<>]*)?([\w`]*)?(<[\w,\s\?]*>)?\s*)");// name
            //     if (std::regex_search(line, match, pattern)) 
            //     {
            //         std::string name = std::string(match[4]) + std::string(match[5])+std::string(match[6]);
            //         if(name.length())
            //         {  
            //             SetAccess(match[1]);
            //             SetName(name);
            //             MatchFuncParamter(name,*this,line);
            //             if(match[3].length())
            //                 SetType(TYPE_SFUNC);
            //             else
            //                 SetType(TYPE_FUNC);
            //             return true;
            //         }
            //     }
            // break;
            case TYPE_SFUNC:// 类函数
            case TYPE_FUNC:// func
            {
                pattern = std::string(REGX_ACCESS);
                std::regex_search(line, match, pattern);
                std::string access = match[1];
                pattern=std::string(R"((static)?\s*func\s+([\+\-\*/=!%&\|<>]*)?([\w`]*)?(<[\w,\s\?]*>)?\s*)");// name
                if (std::regex_search(line, match, pattern)) 
                {
                    std::string name = std::string(match[2]) + std::string(match[3])+std::string(match[4]);
                    if(name.length())
                    {  
                        SetAccess(access);
                        SetName(name);
                        MatchFuncParamter(name,*this,line);
                        if(match[1].length())
                            SetType(TYPE_SFUNC);
                        else
                            SetType(TYPE_FUNC);
                        return true;
                    }
                }
            }   
            break;
            case TYPE_LIAS:// typealias
                pattern = std::string(REGX_ACCESS)
                            +std::string(R"(typealias\s+([\w=\s<>,.:\?]*))");
                if (std::regex_search(line, match, pattern))
                { 
                    if(match[3].length())
                    {
                        SetName(match[3]);
                        SetAccess(match[1]);
                        SetType(type);
                        return true;
                    }
                }
            break;
            case TYPE_IMPORT: // import
                pattern = std::string(R"(\s*import\s+(\w+)\s*)");

                if (std::regex_search(line, match, pattern))
                { 
                    SetName(match[1]);
                    SetType(type);
                    return true;
                }
            break;
            case TYPE_NONE:
                pattern = std::string(R"(^\s*(\{|\}|\s|/)*\s*$)");
                if (std::regex_search(line, match, pattern))
                { 
                    SetName(match[1]);
                    SetType(type);
                    return true;
                }
            break;
            default:
            break;
        }

    }
    catch(const std::regex_error& e)
    {
          ccout.font(f_color::RED) << "Regex["<<type<<":"<<line<<"] error: " << e.what() <<"\n";        
    }
    return false;
}

SwiftPaserType SwiftBase::SwiftTypePaser(const std::string& line)
{
    SwiftPaserType type;
    // ccout<<line<<"\n";
    if("" == line)
        return SwiftPaserType::TYPE_NONE;

    for (type = SwiftPaserType::TYPE_ANNOTATION;type<SwiftPaserType::TYPE_UNKNOW;type = (SwiftPaserType)(type+1)) 
    {
        if(TypeMatch(type,line))
        {
            return type;
        }        
    }

    return SwiftPaserType::TYPE_UNKNOW;
}

 /***********************************
 * SwiftPaser
 * *********************************/
void SwiftContainer::Print(int printlevel)
 {
    if(GetManLevel()>printlevel)
        return;

    TabPrint(GetManLevel(),f_color::RED).fulllines(f_color::BLUE,"",sg_block[8-GetManLevel()])<<"\n";

    TabPrint(GetManLevel(),f_color::RED)<<"[类型]\n";
    TabPrint(GetManLevel(),f_color::GREEN); 
        if(PrintAccess()) PrintSpace();
        if(PrintType()) PrintSpace();
        PrintEnter();
    TabPrint(GetManLevel(),f_color::RED)<<"[名称]\n";
    TabPrint(GetManLevel(),f_color::RED);
        PrintName();
        PrintEnter();
    TabPrint(GetManLevel(),f_color::RED)<<"[版本]\n";
    TabPrint(GetManLevel(),f_color::RED);
        PrintAvailable();
        PrintEnter();
    TabPrint(GetManLevel(),f_color::RED)<<"[继承]\n";
    TabPrint(GetManLevel(),f_color::RED);
        PrintInherit();
        PrintEnter();
    TabPrint(GetManLevel(),f_color::RED)<<"[源码]\n";
    TabPrint(GetManLevel(),f_color::RED);
        PrintContext();
        PrintEnter();
    TabPrint(GetManLevel(),f_color::RED)<<"[注释]\n";
        if(PrintChinese(GetManLevel())) PrintEnter();
        PrintAnnotation(GetManLevel());
        PrintEnter();

    for (int i=0;i<_property.size();++i) 
    {
        _property[i].Print(printlevel);
        if(_property[i].HasChineseInsert())
            SetChineseFg();
    }   
 }

bool SwiftContainer::Paser()
{
    std::string line;
    SwiftBase annotation;
    std::smatch match;
    std::regex pattern(R"(.+\{\s*$)"); 

    line = PopLine(); 
    while(SWIFT_EOF != line)
    {
        SwiftContainer base;
        SwiftPaserType type = base.SwiftTypePaser(line);
        base.SetManLevel(GetManLevel()+1);

        switch(type)
        {
            case TYPE_ANNOTATION:
            case TYPE_AVAILABLE:
                annotation += base;
            break;
            case TYPE_STRUCT:
            case TYPE_CLASS:
            case TYPE_PROTOCOL:
            case TYPE_EXTENSION:
            case TYPE_ENUM:
            case TYPE_SVAR:     
            case TYPE_VAR:
            case TYPE_SLET:
            case TYPE_LET:
            case TYPE_SFUNC:
            case TYPE_FUNC:
            case TYPE_LIAS:
            case TYPE_INIT:
            case TYPE_ASSOCIATE:
            case TYPE_SUBSCRIPT:
            case TYPE_ENUMVALUE:
                base += annotation;
                if (std::regex_search(line, match, pattern))
                { 
                    GetBlock(base.GetLines(),'{','}');
                    if(!base.Paser()) 
                        return false;
                }
                base.SetContext(line);
                Addproperty(base);
                -annotation;
            break;
            default :
                -annotation;
                break;

        } 
        line = PopLine(); 
    }

    return true;
}

 /***********************************
 * SwiftPaser
 * *********************************/
bool Swift::RunPaser(const std::string& file)
{
    std::string line;
    SwiftBase annotation;
    std::smatch match;
    std::regex pattern(R"(.+\.swift$)"); 
    // 校验是否是swift文件
    if (!std::regex_search(file, match, pattern))
    { 
        ccout.font(f_color::RED) << file << " is not swift file!\n";
        return true;
    }
    // 校验文件是否存在
    std::filesystem::path filePath = file;
    if (!std::filesystem::exists(filePath)) 
    {
        ccout.font(f_color::RED) << file << " does not exist.\n"; 
        return false;
    }
    // 获取文件ctime
    auto ftime = std::filesystem::last_write_time(filePath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>
    (
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    _cftime = std::chrono::system_clock::to_time_t(sctp);
    _fileurl = file;
    // 按行分析swift全局对象
    pattern = std::string(R"(.+\{\s*$)");
    ReadFile(file);
    line = PopLine(); 
    ccout.font(f_color::GREEN) << line <<"\n";
    while(SWIFT_EOF != line)
    {
        SwiftContainer base;
        SwiftPaserType type = base.SwiftTypePaser(line);
        base.SetManLevel(1);
        switch(type)
        {
            case TYPE_ANNOTATION:
            case TYPE_AVAILABLE:
                annotation += base;
            break;
            case TYPE_STRUCT:
            case TYPE_CLASS:
            case TYPE_PROTOCOL:
            case TYPE_EXTENSION:
            case TYPE_ENUM:
            case TYPE_SVAR:     
            case TYPE_VAR:
            case TYPE_SLET:
            case TYPE_LET:
            case TYPE_SFUNC:
            case TYPE_FUNC:
            case TYPE_LIAS:
                base += annotation;
                // 匹配{结尾，如果有表示有数据块需要处理
                if (std::regex_search(line, match, pattern))
                { 
                    // 提取{}之间的数据块给base 
                    GetBlock(base.GetLines(),'{','}');
                    if(!base.Paser()) 
                        return false;
                }
                base.SetContext(line);
                AddContainer(base);
                -annotation;
            break;
            default:
                -annotation;
            break;
        } 

        line = PopLine();
        if(type != TYPE_UNKNOW)// 未识别行 程序体，{}，空行等
            ccout.font(f_color::GREEN) << line <<"\n";
    }

    SetInited(true);
    if(!HasData())
        ccout.font(f_color::RED) << file << " is not swift file!\n";
    return true;
}

void Swift::AddContainer(const SwiftContainer& container)
{
    std::string name = container.GetName();
    auto it = _container.find(name);

    if (it != _container.end()) 
    {
        std::vector<SwiftContainer> scontainer;
        scontainer.push_back(container);
        _container[name] = scontainer;
    } 
    else 
    {
        _container[name].push_back(container);
    }
}

std::string& operator << (std::string& out,SwiftContainer& base)
{
    nlohmann::json j;
    nlohmann::json jArray = nlohmann::json::array();

    j["type"] = base.GetType();
    j["access"] = base.GetAccess();
    j["name"] = base.GetName();
    j["inherit"] = base.GetInherit();
    j["available"] = base.GetAvailabe();
    j["context"] = base.GetContext();
    j["level"] = base.GetManLevel();
    j["extend1"] = base.GetExtend1();
    j["extend2"] = base.GetExtend2();
    j["extend3"] = base.GetExtend3();
    j["chinese"] = base.GetChinese();

    for(std::string& it : base.GetAnnotation())
    {
        jArray.push_back(it);
    }
    j["annotation"]= jArray; 

    jArray.clear();
    for(SwiftContainer& it : base.GetProperty())
    {
        std::string jstring;
        jstring << it;
        jArray.push_back(jstring);
    }
    j["property"]= jArray; 

    out = j.dump(4);
    return out;
}

void operator >> (const std::string& jin,SwiftContainer& base)
{
    nlohmann::json j = nlohmann::json::parse(jin);

    base.SetType(j["type"]);
    base.SetAccess(j["access"]);
    base.SetName(j["name"]);
    base.SetInherit(j["inherit"]);
    base.SetAvailabe(j["available"]);
    base.SetContext(j["context"]);
    base.SetManLevel(j["level"]);
    base.SetExtend1(j["extend1"]);
    base.SetExtend2(j["extend2"]);
    base.SetExtend3(j["extend3"]);
    base.SetChinese(j["chinese"]);

    if (j["annotation"].is_array()) 
    {
        for (nlohmann::json::const_iterator it = j["annotation"].begin(); it != j["annotation"].end(); ++it) 
        {
            base.AddAnnotation(*it);
        }
    }

    if (j["property"].is_array()) 
    {
        for (nlohmann::json::const_iterator it = j["property"].begin(); it != j["property"].end(); ++it) 
        {
            SwiftContainer container;
            *it >> container;
            base.Addproperty(container);
        }
    }
}

void Swift::tofile(std::ofstream& out)
{
     nlohmann::json j;

    j["source"] = GetFileUrl();
    j["ctime"] = _cftime;

    for (auto& pair : _container) 
    {
        nlohmann::json jArray = nlohmann::json::array();
        for(SwiftContainer& it : pair.second)
        {
            std::string str;
            str << it;
            jArray.push_back(str);
        }
        j[pair.first] = jArray;
    }
    out << j.dump(4);
}

bool Swift::fromfile(std::ifstream & in)
{
    nlohmann::json j;
    try 
    {
       in >> j;
    } 
    catch (nlohmann::json::parse_error& e) 
    {
        return false;
    }

    SetFileUrl(j["source"]);
    _cftime = j["ctime"];

    for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) 
    {
        if (it.value().is_array()) 
        {
            for (nlohmann::json::const_iterator iit = it.value().begin(); iit != it.value().end(); ++iit) 
            {
                SwiftContainer container;
                *iit >> container;
                AddContainer(container);
            }
        }
    }
    return true;
}


Swift& Swift::operator +(std::string& json)
{
    std::ifstream file(json);
    if (!file.is_open()) 
    {
        std::cerr << "Error["<< errno <<"] opening "<< json << " for reading" << std::endl;
        return *this;
    }

    bool ret = fromfile(file);
    file.close();
    if(!ret)
    {
        ccout.font(f_color::RED) << json << " is not json file\n";
        return *this;
    }

    SetInited(true);

    // 检查源文件ctime
    std::filesystem::path filePath = _fileurl;
    if (!std::filesystem::exists(filePath))     
        return *this;

    auto ftime = std::filesystem::last_write_time(filePath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>
                (ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    if(_cftime != std::chrono::system_clock::to_time_t(sctp))
    {
        ccout.font(f_color::RED) <<_fileurl<<" has changed!\n";
    }

    return  *this;
}

Swift& Swift::operator -()
{
    if(!HasData())
    {
        ccout.font(f_color::RED) <<_fileurl<<" has no data!\n";
        return *this;
    }

    std::filesystem::path filePath = _fileurl;
    std::filesystem::path fileName = filePath.filename();
    std::string jsonfile(SWIFT_JSON_DIRECTORY);
    jsonfile.append(fileName).append(".json");
    ccout.font(f_color::CYAN)<<"Create json...\n";
    std::ofstream file(jsonfile);
    if (!file) 
    {
        std::cerr << "Error["<< errno <<"] opening "<< jsonfile << " for writing" << std::endl;
        return *this;
    }

    tofile(file);
    file.close();
    clear();
    return  *this;
}

 void Swift::Print(const std::string& key,int level)
 {
    if(!HasData())
    {
        ccout.font(f_color::RED) <<" has no data!\n";
        return;
    }

    if (HasKeyWord(key)) 
    {
        for (int i=0;i<_container[key].size();++i) 
        {
            PrintContainer(_container[key][i],level);
        }
    } 
    else 
    {
        ccout.font(f_color::RED) << "not key word "<< key<<"\n";
    }
 }

void Swift::ShowContainerByType(SwiftPaserType type)
{
    bool showallfg = SwiftPaserType::TYPE_UNKNOW == type;
    for (auto& pair : _container)
    {
        std::vector<SwiftContainer>& container = pair.second;
        for(int i = 0;i < container.size();++i)
        {
            if(container[i].GetType() == type || showallfg)
            {
                PrintContainer(container[i],showallfg?7:1);
                char ch = std::cin.get();;
                ccout.command(cc_command::CLS);
            }
        }
    }
}


bool Swift::HasKeyWord(const std::string& key)
{
    auto it = _container.find(key);
    return it != _container.end();
}

void Swift::clear()
{
    _inited = false;
    _haschinesefg = false;
    _fileurl = "";
    _cftime = 0; 
    _container.clear();
}
