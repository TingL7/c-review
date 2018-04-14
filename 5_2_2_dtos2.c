#include <stdio.h>
double m[] = { 3.0039829763669880E-310, 3.0039826351559646E-310 };
void gen()
{
    m[0] = m[0] - m[1];
    puts((char *) m);
}
int main()
{
    gen();
    return 0;
}
