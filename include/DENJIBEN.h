#ifndef DENJIBEN_D
#include<mbed.h>
#define DENJIBEN_D

class Denjiben
{

    private:
    DigitalOut& d_1;

    public:
    Denjiben(DigitalOut&);
    void open();
    void close();
 
};
#endif