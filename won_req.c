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

float64_t calc_exp(float64_t x) {
	long cnt = 0;
	float64_t sum = fp64_sd(0.0);
	float64_t u = calc_powInte(exponentialNum, fp64_round(x));
	if (fp64_isinf(u) == 1) {
		return u;
	}
	float64_t memory = sum;
	while (1) {
		memory = sum;
		//sum = sum + (u / calc_facto(cnt)) * calc_powInte(x - (int)x, cnt);
		sum = fp64_add(sum, fp64_mul(fp64_div(u, calc_facto(fp64_int32_to_float64(cnt))),
			calc_powInte(fp64_sub(x, fp64_round(x)), fp64_int32_to_float64(cnt))));
		if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == -1) break;
		cnt++;
	}
	return sum;
}
