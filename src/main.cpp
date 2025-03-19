#pragma once

#include"Controller.hpp"

int main()
{
    Controller* controller=new Controller();
    controller->global_pre_proccess();
    int count=controller->num_T;
    for(int i=0;i<count;i++)
    {
        controller->run();
    }
}