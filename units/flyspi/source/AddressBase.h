#include <stdexcept>
#include <map>

#define FLYSPI__OCPWRITE 0x80000000

#define DAC__BASEADR 0xc000
#define SLOWADC__BASEADR 0xc000
#define FASTADC__BASEADR 0x3000

#define OCP__CONFROM 0
#define OCP__FASTADC 0x1000
#define OCP__GYROWL 0x2000
#define OCP__ADCCTRL 0x3000
