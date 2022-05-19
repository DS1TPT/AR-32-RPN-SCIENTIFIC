#include <stdio.h>

const double acc = 0.00000000000001;

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
		u = calc_powInte(fp64_sd(-1.0), i);
		d = fp64_add(fp64_sd(i), fp64_sd(1.0));
		d = fp64_mul(fp64_sd(2.0), d);
		u = fp64_mul(u, calc_powInte(x, d));
		d = fp64_add(i, fp64_sd(1.0));
		d = fp64_mul(fp64_sd(2.0), d);
		u = fp64_div(u, calc_facto(d));
		//printf("sum= %f30, u= %f i= %d\n", sum, u, i);
		sum = fp64_add(sum, u);
		i++;
	}
	return sum;
}

double calc_exp(double x, int loop) {
	int i = 0;
	double u = 0;
	double sum = 0;
	//double memory = 1;
	while (i<loop) {
		/*if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}*/
		//memory = sum;
		u = calc_powInte(x, i) / calc_facto(i);
		//printf("sum= %f, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//ino 버전의 함수
float64_t calc_exp(float64_t x, long int loop){
	volatile long int i = 0;
	float64_t u = fp64_sd(0.0);
	float64_t sum = fp64_sd(0.0);
	while (i<loop) {
		float64_t l = fp64_int32_to_float64(i);
		u = fp64_div(calc_powInte(x, l), Calc_facto(l));
		sum = fp64_add(sum, u);
		i++;
	}
	return sum;
}

double calc_powInte(double x, double y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
	double n = x;
	int i = y;
	if (y == 0) {
		n = 1;
	}
	else if (y > 0) {
		while (i != 1) {
			n *= x;
			printf("n= %f, i= %d\n", n, i);
			i--;
		}
	}
	else {
		while (i != 1) {
			n /= x;
			printf("n= %f, i= %d\n", n, i);
			i++;
		}
	}
	return n;
}

float64_t calc_powInte(float64_t x, float64_t y) { 
	float64_t n = x;
	long int i = fp64_int32_to_float64(y);
	if (fp64_compare(y, 0) != 0) {
		n = fp64_sd(1,0);
	}
	else if (fp64_compare(y, 0) == 1) {
		while (i != 1) {
			n = fp64_mul(n, x);
			i--;
		}
	}
	else {
		while (i != 1) {
			n = fp64_div(n, x);
			i++;
		}
	}
	return n;
}

double calc_root(double x) {
	double n = x / 2;
	while (1) {
		double memory = n;
		if (calc_powInte(n, 2) == x) {
			break;
		}
		else {
			n = (calc_powInte(n, 2) + x) / (2 * n);
			printf("root running now n= %Lf, memory= %Lf, bool= %d\n" //테스트용 출력항
				, n, memory, calc_abs(memory - n) < 0.00001);
			if (calc_abs(memory - n) < acc) {
				break;
			}
		}
	}
	return n;
}

double calc_lnA(double x) { //return ln(x+1)
	int cnt = 1;
	double sum = 0.0;
	double memory = 1.0;
	while (1) {
		sum = sum + calc_powInte(-1.0, cnt + 1) * calc_powInte(x, cnt) / cnt;
		printf("A: cnt: %d, sum: %.15Lf, abs: %.15Lf\n", cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		memory = sum;
		cnt++;
	}
	return sum;
}

double calc_ln(double x) {
	double x0;
	if (x >= 0.5 && x <= 1.5) { //lnA에 x-1 대입
		return calc_lnA(x - 1);
	}
	else if (x > 2) {
		x0 = x;
	}
	else { 
		x0 = 1/x;
	}
	int cnt = 0;
	while (x0 >= 2) {
		x0 = calc_root(x0);
		cnt++;
	}
	if (x>2){
		printf("return! : %.15Lf, cnt: %d\n", 1 / x0, cnt);
		return -calc_ln(1 / x0) * calc_powInte(2, cnt);
	}
	else {
		printf("turn! : %.15Lf, cnt: %d\n", 1 / x0, cnt);
		return calc_ln(1 / x0) * calc_powInte(2, cnt);
	}
}

void main(){
	printf("%.15Lf\n ", calc_sin(2, 100));
}
