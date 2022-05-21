#include <fp64lib.h> // 64비트 부동소수점

//c 버전
double calc_inteCut(double x) {
	double x1 = round(x);
	if (x1 <= x) {
		return x -x1;
	}
	else {
		return x - x1 +1.0;
	}
}
//x - round(x)를 대체
//mod와 exp에 쓰임
float64_t calc_inteCut(float64_t x){
	float64_t x1 = fp64_round(x);
	float64_t result = fp64_sub(x, x1);
	if(fp64_compare(x1, x) <= 0){
		return result;
	}
	else{
		return fp64_add(result, fp64_sd(1.0));
	}
}
//실수 범위 모듈러 연산
//sin연산에 쓰임
float64_t calc_mod(float64_t x, float64_t y) {
	float64_t x0 = fp64_div(x, y);
	float64_t x1 = calc_inteCut(x0);
	return fp64_mul(x1, y);
}

float64_t calc_exp(float64_t x) {
	long cnt = 0;
	float64_t sum = fp64_sd(0.0);
	float64_t u = calc_powInte(exponentialNum, fp64_sub(x, calc_inteCut(x)));
	if (fp64_isinf(u) == 1) {
		return u;
	}
	float64_t memory = sum;
	while (1) {
		memory = sum;
		//sum = sum + (u / calc_facto(cnt)) * calc_powInte(calc_inteCut(x), cnt);
		sum = fp64_add(sum, fp64_mul(fp64_div(u, calc_facto(fp64_int32_to_float64(cnt))),
			calc_powInte(calc_inteCut(x), fp64_int32_to_float64(cnt))));
		if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == -1) break;
		cnt++;
	}
	return sum;
}
