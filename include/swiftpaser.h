#ifndef _SWIFT_PASER_H_
#define _SWIFT_PASER_H_
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <iostream>
#include <algorithm>
#include <map>
#include <nlohmann/json.hpp>

#define SWIFT_EOF "SWIFTEOF"

#ifndef SWIFT_JSON_DIRECTORY
#define SWIFT_JSON_DIRECTORY "~/swifttranslater/"
#endif
// 保存对象使用的源码
class LinesBuffer
{
public:
    void ReadFile(const std::string& fileurl) 
    {
        std::ifstream file(fileurl);
        if (!file.is_open()) 
        {
            std::cerr << "Failed to open the file." << std::endl;
            return;
        }
        _content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }

    std::string& GetLine(){return _line;}
    std::string& PopLine()
    {
        std::istringstream stream(_content);
        if(!std::getline(stream, _line))
        {
            _line = SWIFT_EOF;
        }
        else
        {
            _content.erase(0,_line.length()+1);
        }
        return GetLine();
    }
    std::string& GetLines(){return _content;};
    void GetBlock(std::string& buffer,const char blockleft,const char blockright,int hadleftcount=1);

private:
    std::string _line;
    std::string _content;
};

typedef enum:int
{
    TYPE_ANNOTATION=0,
    TYPE_AVAILABLE,//1

    TYPE_STRUCT,//2
    TYPE_CLASS,//3
    TYPE_PROTOCOL,//4
    TYPE_ASSOCIATE,//5
    TYPE_EXTENSION,//6
    TYPE_ENUM,//7
    TYPE_ENUMVALUE,//8

    TYPE_SVAR,//9      
    TYPE_VAR,//10
    TYPE_SLET,//11
    TYPE_LET,//12
    
    TYPE_SUBSCRIPT,//13

    TYPE_SFUNC,//14
    TYPE_FUNC,//15

    TYPE_LIAS,//16

    TYPE_INIT,//17
    
    TYPE_IMPORT,//18

    TYPE_NONE,//19
    TYPE_UNKNOW//20
}SwiftPaserType;

class SwiftBase
{
public:
    void Print();

    SwiftPaserType SwiftTypePaser(const std::string& line);

    virtual void SetChineseFg(){};

    void SetType(SwiftPaserType type){_type=type;}
    SwiftPaserType GetType()const {return _type;}

    void SetAccess(const std::string access){_access=access;}
    std::string GetAccess()const {return _access;}

    void SetName(const std::string& name){_name=name;}
    std::string GetName()const {return _name;} 

    void AddAnnotation(const std::string& annotation){_annotation.push_back(annotation);}
    void SetAnnotation(std::vector<std::string>& annotation){_annotation=annotation;}
    std::vector<std::string>& GetAnnotation() {return _annotation;}

    void SetChinese(const std::string& chinese){_annotation_cn=chinese;}
    std::string& GetChinese(){return  _annotation_cn;}

    void SetInherit(const std::string& inherit){_inherit=inherit;}
    std::string& GetInherit(){return  _inherit;}

    void SetAvailabe(const std::string & availale){_available=availale;}
    std::string& GetAvailabe(){return  _available;}

    void SetExtend1(const std::string& extend){_extend1=extend;}
    std::string& GetExtend1(){return  _extend1;}

    void SetExtend2(const std::string& extend){_extend2=extend;}
    std::string& GetExtend2(){return  _extend2;}

    void SetExtend3(const std::string& extend){_extend3=extend;}
    std::string& GetExtend3(){return  _extend3;}

    void SetManLevel(int level){_manleve = level;}
    int GetManLevel()const {return _manleve;}

    void SetContext(const std::string& line) {_context.append(line);}
    std::string& GetContext() {return _context;}


    // 实现版本和注释的拷贝
    SwiftBase& operator += (SwiftBase& base)
    {
        std::vector<std::string>& b = base.GetAnnotation();
        for(int i=0;i<b.size();++i) AddAnnotation(b[i]);
        _available.append(base.GetAvailabe());
        return *this;
    }
    SwiftBase& operator - ()
    {
        _annotation.clear();
        _available.clear();
        return *this;
    }

    bool PrintName() const;
    bool PrintType()const;
    bool PrintInherit() const;
    bool PrintAnnotation(int level=1) const;
    bool PrintAccess() const;
    bool PrintExtend1() const;
    bool PrintExtend2() const;
    bool PrintExtend3() const;
    bool PrintContext() const;
    bool PrintAvailable() const;
    bool PrintChinese(int level=1);

private:
    bool TypeMatch(SwiftPaserType type,const std::string& line);
    SwiftPaserType _type=TYPE_UNKNOW;
    std::string _access="public";
    std::string _name = "";
    std::vector<std::string> _annotation;//注释
    std::string _annotation_cn;
    std::string _inherit;//继承关系
    std::string _available;//版本
    std::string _extend1;// return
    std::string _extend2;// param
    std::string _extend3;// where
    std::string _context;

    int _manleve = 1;
};


class SwiftContainer:public SwiftBase,public LinesBuffer
{
public:
    bool Paser();
    // 获取属性列表
    void Addproperty(const SwiftContainer& property){_property.push_back(property);}
    std::vector<SwiftContainer>& GetProperty() {return _property;}
    // 名称和版本匹配
    bool operator == (SwiftContainer& base)const {return ((GetAccess()==base.GetAccess())&&(GetName()==base.GetName()));}
    // 仅名称匹配
    bool operator -= (SwiftContainer& base)const {return (GetName()==base.GetName());}
    // 依据level打印
    void Print(int printlevel=1);
    // 重载，通知需要保存
    void SetChineseFg(){_haschinesefg = true;};
    bool HasChineseInsert() 
    { 
        bool ret = _haschinesefg; 
        _haschinesefg = false;
        return ret;
    };

private:
    bool _haschinesefg = false;
    std::vector<SwiftContainer> _property;
};

std::string& operator << (std::string& out,SwiftContainer& base);
void operator >> (const std::string& jin,SwiftContainer& base);

class Swift:public LinesBuffer
{
public:
    Swift(const std::string fileurl):_fileurl(fileurl){};
    Swift()=default;
    // 分析源码
    bool RunPaser() {return RunPaser(_fileurl);};
    bool RunPaser(const std::string& fileurl);
    // 读取json数据
    Swift& operator +(std::string& json);
    // 保存json数据
    Swift& operator -();
    // 设置源码路径
    std::string GetFileUrl()const {return _fileurl;};
    void SetFileUrl(const std::string& url){_fileurl = url;}
    // 是否数据
    bool HasData()const  {return _inited;}
    // 打印属性
    void Print(const std::string& key,int level);
    void SetChineseFg() {_haschinesefg = true;}
    // 根据类型显示全局属性
    void ShowContainerByType(SwiftPaserType type = SwiftPaserType::TYPE_UNKNOW);
    // 查询
    bool HasKeyWord(const std::string&);

    void clear();
    ~Swift() 
    {
        if(_haschinesefg)
            -*this;
    };
private:

    void tofile (std::ofstream& out);
    bool fromfile(std::ifstream & in);

    void AddContainer(const SwiftContainer& container);
    void PrintContainer(SwiftContainer& container,int level=1)
    {
         container.Print(level);
        if(container.HasChineseInsert())
            SetChineseFg();
    }

    void SetInited(bool inited)
    {
        if(inited&&_container.size())
            _inited = true;
        else
            _inited = false;
    }
    // 数据已经更新，可以读取
    bool _inited = false;
    // 有新的中文注释，需要更新json
    bool _haschinesefg = false;
    // 源文件路径
    std::string _fileurl;
    // 源文件ctime
    std::time_t _cftime; 
    std::map<std::string,std::vector<SwiftContainer>> _container;  
};


#endif