//
//  int_large.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/14/25.
//

#include "int_large.h"

/* // long long
 long long random_il(std::mt19937& rng){ // random long long
    long long a = rng();
    long long b = rng();
    return ((a/2) << 32)+b;
}

long long ilround(double x){
    return std::llround(x);
}

long long ilround(float x){
    return std::llround(x);
}
*/

int_large random_il(std::mt19937& rng,int pows_32 = 2){
    int_large a = 0;
    for (int i = 0; i<pows_32; i++){
        a*=pow_ui(2,32);// mt19937 max output is 2^32-1
        a+=rng();
    }
    return a;
}

int_large iltrunc(double x){
    return mpz_class(x);
}

int_large iltrunc(float x){
    return mpz_class(x);
}

int_large trunc_root(const int_large & input, unsigned long int n){
    int_large output;
    mpz_root (output.get_mpz_t(), input.get_mpz_t(), n);
    return output;
}

int int_log232(const int_large & input){ // log abs actually when I get around to it
    if (input < 0){
        return (int)((input.get_str(2).length() -2)/32);
    } else {
        return (int)((input.get_str(2).length() -1)/32);
    }
}

int_large random_il_below (std::mt19937 & rng, const int_large & bound){
    return ((random_il(rng,int_log232(bound)+1))%bound);
}

int_large pow_ui(const int_large & input, unsigned int exp){
    int_large output;
    mpz_pow_ui (output.get_mpz_t(), input.get_mpz_t(), exp);
    return output;
}
