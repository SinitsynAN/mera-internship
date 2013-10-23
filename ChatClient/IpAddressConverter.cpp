#include "IpAddressConverter.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

IpAddressConverter::IpAddressConverter()
{
    address = 0;
    for (int i = 0; i < NUMBS_IN_IP; i++) {
        addressBuffer[i] = 0;
    }
    isAddressValid = false;
}

bool IpAddressConverter::TryConvert(char adr[])
{
    address = 0;
    isAddressValid = false;
    
    if (strlen(adr) != IP_LENGTH) {
        perror("Invalid address length\n");
        return isAddressValid;
    }

    int k = 0;
    for (int i = 0; i < NUMB_OF_INTERVALS; i++) {
        for (int j = 0; j < NUMBS_IN_INTERVAL; j++) {
            if (!isdigit(adr[k])) {
                perror("Invalid character in address\n");
                return isAddressValid;
            }
            addressBuffer[NUMBS_IN_INTERVAL * i + j] = adr[k];
            k++;
        }
        if (k < IP_LENGTH) {
            if (adr[k] == '.')
                k++;
            else
            {
                perror("Invalid character in address\n");
                return isAddressValid;
            }
        }
    }

    int octet;
    int multiplier;
    for (int i = 0; i < NUMB_OF_INTERVALS; i++) {
        octet = 0;
        multiplier = 1;
        for (int j = 0; j < NUMBS_IN_INTERVAL; j++)
            octet = octet * 10 + addressBuffer[NUMBS_IN_INTERVAL * i + j] - '0';
        for (int k = 0; k < NUMB_OF_INTERVALS - i - 1; k++)
            multiplier *= 256;
        address += octet * multiplier;
    }
    
    return isAddressValid = true;
}

bool IpAddressConverter::IsAddressValid()
{
    return isAddressValid;
}

unsigned long IpAddressConverter::GetAddress()
{
    return address;
}
