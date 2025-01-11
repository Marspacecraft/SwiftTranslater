# SwiftTranslater
 swift源文件注释翻译   
 采用C++编写
 采用命令行运行方式
 调用谷歌翻译接口翻译

# 安装
- 安装 [nlohmann-json](https://github.com/nlohmann/json)  
- 安装 python3  
- 安装命令  
`mkdir build && cd build`  
`cmake ..`  
`make`  
`make install`  
# 用法  
## 分析文件  
`swifttrans -p SwfitUI.swift`  
## 查询关键字  
`swifttrans "withTransaction<Result>"`  
![结果](https://github.com/Marspacecraft/SwiftTranslater/blob/main/pic.png)   
# 其他   
## Debug版本    
- cmake默认生成Release版本Makefile  
- `make debug`调用cmake重新生成Makefile，后面make的都是debug版本
## 单元测试  
`cmake .. -DGTEST=ON`

