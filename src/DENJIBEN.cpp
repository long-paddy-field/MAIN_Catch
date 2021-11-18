#include<mbed.h>
#include<DENJIBEN.h>

Denjiben::Denjiben(DigitalOut& d1)
:d_1(d1)
{

}

void Denjiben::open()
{
    d_1 = 0;
}

void Denjiben::close()
{
    d_1 = 1;
}