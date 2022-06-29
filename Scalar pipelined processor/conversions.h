#ifndef CONVERSIONS_H
#define CONVERSIONS_H

using namespace std;


string int_to_hex(int x)
{
    string s = "";
    int flag = 0;

    /*
    if(x<0)
    {
        x*=(-1);
        x = pow(2,8) - x;
    }
    */

    int data1 = x/16;
    int data2 = x - 16*data1;

    if(data1>9)
    {
        char temp = data1+87;
        s+=temp;
    }
    else
    {
        char temp = data1+48;
        s+=temp;
    }

    if(data2>9)
    {
        char temp = data2+87;
        s+=temp;
    }
    else
    {
        char temp = data2+48;
        s+=temp;
    }
    
    return s;
}

int hex_to_int(string data)
{
    int ans = 0;
    
    if(data[0]>57)
        ans = 16*(data[0]-87);
    else
        ans = 16*(data[0]-48);

    if(data[1]>57)
        ans += (data[1]-87);
    else
        ans += (data[1]-48);

    /*
    if(ans>pow(2,7)-1)
    {
        ans = ans - pow(2,8);
    }
    */

    return ans;
}









string twos_complement_int_to_hex(int x)
{
    string s = "";
    int flag = 0;

    
    if(x<0)
    {
        x*=(-1);
        x = pow(2,8) - x;
    }

    int data1 = x/16;
    int data2 = x - 16*data1;

    if(data1>9)
    {
        char temp = data1+87;
        s+=temp;
    }
    else
    {
        char temp = data1+48;
        s+=temp;
    }

    if(data2>9)
    {
        char temp = data2+87;
        s+=temp;
    }
    else
    {
        char temp = data2+48;
        s+=temp;
    }
    
    return s;
}

int twos_complement_hex_to_int(string data)
{
    int ans = 0;
    
    if(data[0]>57)
        ans = 16*(data[0]-87);
    else
        ans = 16*(data[0]-48);

    if(data[1]>57)
        ans += (data[1]-87);
    else
        ans += (data[1]-48);

    
    if(ans>pow(2,7)-1)
    {
        ans = ans - pow(2,8);
    }

    return ans;
}


#endif