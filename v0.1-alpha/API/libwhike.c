#include "../include/HAL/alog.h"
#include <stdint.h>
#include <time.h>

typedef struct {
	double result;
	int error;
} CalcRes;

double afabs(double x) {
	return x < 0 ? -x : x;
}

double asqrt(double x) {
	if (x < 0) return -1.0;
	if (x == 0) return 0.0;
	double res = x;
	double prev = 0;
	while (afabs(res - prev) > 1e-7) {
		prev = res;
		res = 0.5 * (res + x / res);
	}
	return res;
}

double apow(double base, int exp) {
	if (exp== 0) return 1.0;
	double res = 1.0;
	int abs_exp = (exp < 0) ? -exp : exp;
	for (int i = 0; i < abs_exp; i++) {
		res *= base;
	}
	return exp < 0 ? (1.0 / res) : res;
}

double asin(double x) {
	while (x > 3.141592653589793) x -= 6.283185307179586;
	while (x < -3.141592653589793) x += 6.283185307179586;
    	double sum = x;
    	double term = x;
    	for (int i = 1; i < 10; i++) {
        	term = -term * x * x / ((2 * i) * (2 * i + 1));
        	sum += term;
    	}
    	return sum;
}

double acos(double x) {
    while (x > 3.141592653589793) x -= 6.283185307179586;
    while (x < -3.141592653589793) x += 6.283185307179586;
    double sum = 1.0;
    double term = 1.0;
    for (int i = 1; i < 10; i++) {
        term = -term * x * x / ((2 * i - 1) * (2 * i));
        sum += term;
    }
    return sum;
}

double atan(double x) {
    double c = acos(x);
    if (afabs(c) < 1e-9) return 0.0;
    return asin(x) / c;
}

double alog(double x) {
    if (x <= 0) return -1.0;
    double num = x - 1.0;
    double den = x + 1.0;
    double term = num / den;
    double term_sq = term * term;
    double sum = 0.0;
    double denom_coeff = 1.0;
    for (int i = 0; i < 10; i++) {
        sum += term / denom_coeff;
        term *= term_sq;
        denom_coeff += 2.0;
    }
    return 2.0 * sum;
}

uint64_t atick_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)(ts.tv_sec * 1000000000ULL + ts.tv_nsec);
}

uint64_t atick_us(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)(ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL);
}

uint64_t atick_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
}

uint64_t atick_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec;
}
