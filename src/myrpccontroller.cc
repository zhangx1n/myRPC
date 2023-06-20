#include "myrpccontroller.h"

myrpcController::myrpcController()
{
    m_failed = false;
    m_errText = "";
}

void myrpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool myrpcController::Failed() const
{
    return m_failed;
}

std::string myrpcController::ErrorText() const
{
    return m_errText;
}

void myrpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

// 目前未实现具体的功能
void myrpcController::StartCancel(){}
bool myrpcController::IsCanceled() const {return false;}
void myrpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}