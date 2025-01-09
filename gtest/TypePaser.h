
#ifndef _T_TYPE_PASER_H_
#define _T_TYPE_PASER_H_
#include <gtest/gtest.h>
#include "test_main.h"
#include "swiftpaser.h"



TEST(TypePaser,Annotation1)
{
    SwiftBase base;
    std::string str("/");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_NONE);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),str),false);
}
TEST(TypePaser,Annotation2)
{
    SwiftBase base;
    std::string str("//");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string("")),true);
}
TEST(TypePaser,Annotation3)
{
    SwiftBase base;
    std::string str("///////////");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string("")),true);
}
TEST(TypePaser,Annotation4)
{
    SwiftBase base;
    std::string str("  /////////// ");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string(" ")),true);
}

TEST(TypePaser,Annotation5)
{
    SwiftBase base;
    std::string str("  //abcd\n ");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string("abcd")),true);
}
TEST(TypePaser,Annotation6)
{
    SwiftBase base;
    std::string str("  ///abcd ");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string("abcd ")),true);
}
TEST(TypePaser,Annotation7)
{
    SwiftBase base;
    std::string str("  /// abcd ");

    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string(" abcd ")),true);
}

TEST(TypePaser,available1)
{
    SwiftBase base;
    std::string str("available");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_UNKNOW);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"");
}

TEST(TypePaser,available2)
{
    SwiftBase base;
    std::string str("@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_AVAILABLE);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *");
}

TEST(TypePaser,available3)
{
    SwiftBase base;
    std::string str("@available (iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_AVAILABLE);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *");
}

TEST(TypePaser,available4)
{
    SwiftBase base;
    std::string str(" @available (iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_AVAILABLE);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *");
}

TEST(TypePaser,available5)
{
    SwiftBase base;
    std::string str(" @availableiOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_UNKNOW);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"");
}
TEST(TypePaser,available6)
{
    SwiftBase base;
    std::string str(" @ available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_UNKNOW);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"");
}

TEST(TypePaser,available7)
{
    SwiftBase base;
    std::string str("//@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)");
    
    ASSERT_EQ(base.SwiftTypePaser(str),SwiftPaserType::TYPE_ANNOTATION);
    ASSERT_EQ(SecrchVect<std::string>(base.GetAnnotation(),std::string("@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)")),true);
    ASSERT_STREQ(base.GetAvailabe().c_str(),"");
}





#endif


