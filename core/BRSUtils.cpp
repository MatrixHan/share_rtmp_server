#include "BRSUtils.h"


namespace BRS 
{
long long int getTimeDec()
{
	register uint32_t lo, hi;
	register unsigned long long o;
    __asm__ __volatile__ (
        "rdtscp" : "=a"(lo), "=d"(hi)
        );
	o = hi;
	o <<= 32;
	return (o | lo);
}
BString now()
{
    return timeFormat(getTimeDec());
}

}