#include <iostream>
#include <map>
#include <filesystem>
#include "swiftpaser.h"
#include  "ccout.h"
#include "debug.h"

Swift swift;
typedef void (* argfunc)(int argc,char* argv[]);

typedef struct 
{
    std::string parameter;
    argfunc func;
    std::string explanation;
}tArgFunc;


void Usage(int argc,char* argv[]);
void PaserSwiftFile(int argc,char* argv[]);
void DebugMode(int argc,char* argv[]);

static tArgFunc sg_ArgFunc[] =
{
    {"-d",DebugMode,"`-d (cmd)`:goto debug mode with cmd"},
    {"-p",PaserSwiftFile,"`-p [swift file url]`:paser swift file and create json file"},
    {"-h",Usage,"`-h`:show usage"},
};

void Usage(int argc,char* argv[])
{
    ccout.font(f_color::RED) << "Usage:\n";

     ccout.font(f_color::GREEN) << "\t" 
            << "string"  << "\t" 
                 <<  ccout.font(f_color::NONE) <<  "`[code keyword]`:show the keyword explanations\n";

    for(int i=0;i<sizeof(sg_ArgFunc)/sizeof(tArgFunc);++i)
    {
        ccout.font(f_color::GREEN) << "\t" 
            << sg_ArgFunc[i].parameter  << "\t" 
                <<  ccout.font(f_color::NONE) << sg_ArgFunc[i].explanation <<"\n";
    }
}

void DebugMode(int argc,char* argv[])
{
    if(3 <= argc)
        Debug(argv[2]);
    else
        Debug("");
}


void PaserSwiftFile(int argc,char* argv[])
{
    if(argc <3 )
    {
        ccout.font(f_color::RED)<<"Not input file url\n";
        Usage(argc,argv);
        return;
    }
    std::string url = argv[2];
    ccout.font(f_color::CYAN)<<"Paser file...\n";
    swift.RunPaser(url);  
    -swift;
}

void Search(std::string keyword)
{
    std::vector<std::string> JsonFiles;
    try 
    {
        for (const auto& entry : std::filesystem::directory_iterator(SWIFT_JSON_DIRECTORY)) 
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json") 
            {
                JsonFiles.push_back(entry.path().filename().string());
            }
        }
    } 
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "General error: " << e.what() << std::endl;
    }

    for(int i=0;i<JsonFiles.size();++i)
    {
        swift + std::string(SWIFT_JSON_DIRECTORY).append(JsonFiles[i]);
        if(swift.HasKeyWord(keyword))
        {
            swift.Print(keyword,1);
            return;
        }
        swift.clear();
    }
}

#ifndef GOOGLE_TEST
int main(int argc,char* argv[])
{
    if(argc < 2)
    {
        ccout.font(f_color::RED)<<"parameters not enough\n";
        Usage(argc,argv);
        return 0;
    }

    for(int i=0;i<sizeof(sg_ArgFunc)/sizeof(tArgFunc);++i)
    {
        if(argv[1] == sg_ArgFunc[i].parameter)
        {
            sg_ArgFunc[i].func(argc,argv);
            return 0;
        }
    }
    Search(argv[1]);
    return 0;
}
#endif
