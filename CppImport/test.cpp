#include "test.h"
//#include <iostream>
//TEst comment

int NOCLASSFIELD;

class Testing {
public:
    int i;
    Testing(){}
    int myFunction(int argument1, bool argument2){
        int k = 20;
        if(k == 10)
        {
            int l = 10;
            k=20;
        }
        else
        {
            int f=0;
            k=0;
        }
        return k;}
};


class Another {
    //COMMENT IN CLAS
    bool aBoolVar;
    void aVoidFunc(){
        bool aNotherBoolVar;};
};


class OneMore{
    int oneMoreFunction(){
        int oneMoreVariable, oneMoreVar2;
        return 100;
    }
};

int Header::headerFunc()
{
    int someVarInCPP;
    return 0;
}

int Header::headerFunc(int k)
{
    int anotherVar;
    return 100;
}
