#include <stdio.h>

double calc_facto(double x) {
	if (x == 0) {
		return 1;
	}
	else {
		double sum = 1;
		for (double i = 2; i <= x; i++) {
			sum = sum * i;
		}
		return sum;
	}
}

double calc_powInte(double x, double y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
	double n = x;
	if (y == 0) {
		n = 1;
	}
	else if (y > 0) {
		for (int i = 1; i < y; i++) {
			n *= x;
			//printf("i = %d\n", i);
		}
	}
	else {
		for (int i = 1; i > y; i--) {
			n = n / x;
			//printf("i = %d\n", i);
		}
	}
	return n;
}

double calc_sin(double x, int loop) { // 0~2pi 내의 연산으로 바꾸자.
	double sum = 0;
	double u = 0;
	volatile int i = 0;
	double memory = 1;
	while (i<loop) {
		/*if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}*/
		memory = sum;
		u = calc_powInte(-1, i) * calc_powInte(x, 2 * i + 1) / calc_facto(2 * i + 1);
		//printf("sum= %f30, u= %f i= %d\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//ino 버전의 함수
float64_t calc_sin(float64_t x, int loop){
	float64_t sum = fp64_sd(0.0);
	float64_t u = fp64_sd(0.0);
	float64_t d = fp64_sd(1.0);
	volatile int i = 0;
	float64_t memory = fp64_sd(1.0);
	while (i<loop) {
		/*if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}*/
		memory = sum; 
		u = fp64_mul(calc_powInte(fp64_sd(-1.0), i), calc_powInte(x, fp64_add(fp64_mul(fp64_sd(2.0), fp64_sd(i)), fp64_sd(1.0))));
		d = calc_facto(fp64_add(fp64_mul(fp64_sd(2.0), i), fp64_sd(1.0)));
		//printf("sum= %f30, u= %f i= %d\n", sum, u, i);
		sum = sum + u/d;
		i++;
	}
	return sum;
}

void main(){
	printf("%.15Lf\n ", calc_sin(2, 100));
}
