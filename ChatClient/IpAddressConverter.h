#ifndef IPADDRESSCONVERTER_H
#define	IPADDRESSCONVERTER_H

#define IP_LENGTH 15
#define NUMBS_IN_IP 12
#define NUMB_OF_INTERVALS 4
#define NUMBS_IN_INTERVAL 3

class IpAddressConverter
{
private:
    unsigned long address;
    char addressBuffer[];
    bool isAddressValid;
    
public:
    IpAddressConverter();
    bool TryConvert(char adr[]);
    bool IsAddressValid();
    unsigned long GetAddress();
};

#endif