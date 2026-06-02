#include <Server/Boostrap.h>
#include <Server/Server.h>

int Online::Server::Execute(int argc, char** argv)
{
    return Online::Runtime::Server::Instance().Execute();
}

void Online::Server::Terminate()
{
    Online::Runtime::Server::Instance().Terminate();
}