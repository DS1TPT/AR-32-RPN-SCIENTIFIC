#include <stdio.h>
#include <fp64lib.h> // 64비트 부동소수점

//실수 범위 모듈러 연산
//sin연산에 쓰임
float64_t calc_mod(float64_t x, float64_t y) {
	float64_t x0 = fp64_div(x, y);
	float64_t x1 = fp64_round(x0);
	float64_t x2;
	if (fp64_compare(x1, x0) <= 0) {
		x2= fp64_sub(x0, x1);
	}
	else {
		x2 = fp64_add(fp64_sub(x0, x1), fp64_sd(1.0));
	}
	return fp64_mul(x2, y);
}

