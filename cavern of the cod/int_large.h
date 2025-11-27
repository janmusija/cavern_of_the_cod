//
//  int_large.h
//  ominous dice torment game
//
//  Created by jan musija on 9/14/25.
//

#ifndef int_large_h
#define int_large_h

#include <stdio.h>
#include <random>
#include <vector>
#include <gmp.h>
#include <gmpxx.h>

typedef mpz_class int_large; // will replace later with something equivalent. used for xp, coins, hp, etc

/*
long long random_il(std::mt19937& rng); // random long long
// max value = 2^63 - 1
// min value = 0

long long ilround(double x);

long long ilround(float x);
 */
int_large random_il(std::mt19937& rng,int pows_32);

int_large random_il_below (std::mt19937 & rng, const int_large & bound);

int_large iltrunc(double x);

int_large iltrunc(float x);

int_large trunc_root(const int_large & input, unsigned long int n);

int int_log232(const int_large & input);

int_large pow_ui(const int_large & input, unsigned int exp);

#endif /* int_large_h */
