#include <iostream>
#include "AcorpSprinter56kExt.h"

using namespace std;

int main()
{
    AcorpSprinter56kExt::AcorpSprinter56kExt mdm("COM2");
    mdm.init();
    mdm.WaitPhoneCall();
    return 0;
}