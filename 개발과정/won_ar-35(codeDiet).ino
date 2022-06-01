#include <stdio.h>
/*보려는 소수점 아래 숫자보다 다소 여유롭게 ACCURACY를 설정하는 것이 좋다.*/
#define ACCURACY 0.00000000000001

const double pi = 3.141592653589793238;

//x의 절댓값을 내보내는 함수
double calc_abs(double x) {
	if (x > 0) {
		return x;
	}
	else {
		return -x;
	}
}

//x!을 출력하는 함수
//x는 0보다 크거나 같은 정수 범위
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

/* x^y 값을 내보내는 함수
x는 0보다 크거나 같은 실수 범위 y는 정수 범위*/
double calc_powInte(double x, double y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
	double n = x;
	if (y == 0) {
		n = 1;
	}
	else if (y > 0) {
		for (int i = 1; i < y; i++) {
			n *= x;
		}
	}
	else {
		for (int i = 1; i > y; i--) {
			n = n / x;
		}
	}
	return n;
}

double calc_exp(double x) {
	double i = 0;
	double u = 0;
	double sum = 0;
	double memory = 1;
	while (1) {
		if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = calc_powInte(x, i) / calc_facto(i);
		//printf("sum= %f, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

double calc_ln(double a) {
	double n = 1;
	while (1) {
		double memory = n;
		n = n - (calc_exp(n) - a) / calc_exp(n);//뉴튼 랩튼법
		//printf("test n= %f, memory= %f, f(x)= %f, dy/dx= %f\n", n, memory, calc_exp(n) - a, calc_exp(n)); //테스트용 출력항
		if (calc_abs(memory - n) < ACCURACY) {
			break;
		}
	}
	return n;
}

/*x^y를 출력하는 함수
x는 0보다 크거나 같은 실수 범위, y는 실수 범위*/
double calc_pow(double x, double y) {
	double YlnX = y * calc_ln(x);
	return calc_exp(YlnX);
}

//sinx를 출력하는 함수
//x는 실수 범위
double calc_sin(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1) {
		if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = calc_powInte(-1, i) * calc_powInte(x, 2 * i + 1) / calc_facto(2 * i + 1);
		//printf("sum= %f30, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//cosx를 출력하는 함수
//x는 실수 범위
double calc_cos(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1) {
		if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = calc_powInte(-1, i) * calc_powInte(x, 2 * i) / calc_facto(2 * i);
		//printf("sum= %f, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//tanx를 출력하는 함수
//x는 실수 범위
double calc_tan(double x) {
	return calc_sin(x) / calc_cos(x);
}

double calc_arcsin(double x) {
	double i = 0;
	double u = 0;
	double sum = 0;
	double memory = 1;
	while (1) {
		if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = calc_facto(2 * i) * calc_powInte(x, 2 * i + 1) / (calc_powInte(4, i) * calc_powInte(calc_facto(i), 2) * (2 * i + 1));
		//printf("sum= %f, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

double calc_arccos(double x) {
	return pi / 2 - calc_arcsin(x);
}

double calc_arctan(double x) {
	double i = 0;
	double u = 0;
	double sum = 0;
	double memory = 1;
	while (1) {
		if (calc_abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = calc_powInte(-1, i) * calc_powInte(x, 2 * i + 1) / (2 * i + 1);
		//printf("sum= %f, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

double calc_log(double a) {
	return calc_ln(a) / calc_ln(10);
}

double calc_sqrt(double x) {
	return calc_pow(x, 1.0 / 2);
}

double calc_sqrtY(double x, double y) {
	return calc_pow(x, 1.0 / y);
}

double calc_log2(double x) {
	return calc_ln(x) / calc_ln(2);
}

double calc_logXY(double x, double y) {
	return calc_ln(y) / calc_ln(x);
}

double calc_radToDegree(double a) {
	return a * 180 / pi;
}

double calc_degreeToRad(double a){
	return a * pi / 180;
}

double calc_radToGon(double a) {
	return a / pi * 200;
}

double calc_gonToRad(double a) {
	return a * pi / 200;
}

void main() {
	printf("log= %.15Lf\npow= %.15Lf\nsin= %.15f\ncos= %.15Lf\narccos= %.15Lf\narctan= %.15Lf\nroot= %.15Lf\n"
		, calc_log(0.4), calc_pow(3.234, 5.3244), calc_sin(11.34), calc_cos(0.1), calc_arccos(0.1), calc_arctan(0.2), calc_sqrt(4.5));
}
