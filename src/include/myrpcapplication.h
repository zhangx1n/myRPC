#pragma once

#include "myrpcconfig.h"
#include "myrpcchannel.h"
#include "myrpccontroller.h"

// myrpc框架的基础类，负责框架的一些初始化操作
class myrpcApplication
{
public:
    static void Init(int argc, char **argv);
    static myrpcApplication& GetInstance();
    static myrpcConfig& GetConfig();
private:
    static myrpcConfig m_config;

    myrpcApplication(){}
    myrpcApplication(const myrpcApplication&) = delete;
    myrpcApplication(myrpcApplication&&) = delete;
};