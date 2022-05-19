#include <stdio.h>

const double pi = 3.14159265359;

const double acc = 0.00000000000001;
const double loop = 1000;

int facto = 0;
int powInte = 0;

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
		facto++;
	}
	else {
		double sum = 1;
		for (double i = 2; i <= x; i++) {
			sum = sum * i;
			facto++;
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
		powInte++;
	}
	else if (y > 0) {
		for (int i = 1; i < y; i++) {
			n *= x;
			powInte++;
		}
	}
	else {
		for (int i = 1; i > y; i--) {
			n = n / x;
			powInte++;
		}
	}
	return n;
}

double calc_root(double x) {
	double n = x / 2;
	double m0;
	int cnt = 0;
	double outMemory = 0.0; 
	double memory = 0.0;
	while (1) {
		double memory = n;
		if (cnt % 2 == 1) {
			printf("true 1 : cnt : %d\n", cnt);
			if (outMemory == n){
				printf("true 2 : outMemory: %.15Lf \n", outMemory);
				break;
			}
			outMemory = n;
		}
		m0 = calc_powInte(n, 2);
		if (m0 == x) {
			break;
		}
		else {
			n = (m0 + x) / (2 * n);
			printf("root running now n= %.15Lf, memory= %.15Lf, outMemory: %.15Lf, bool= %d\n" //테스트용 출력항
				, n, memory, outMemory, calc_abs(memory - n) < acc);
			if (calc_abs(memory - n) < acc) {
				break;
			}
			cnt++;
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
		printf("lnA: cnt: %d, sum: %.15Lf, abs: %.15Lf\n", cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		memory = sum;
		cnt++;
	}
	return sum;
}

double calc_lnB(double x, int loop) { //return ln(x) -ln(x-1)
	int cnt = 1;
	double sum = 0.0;
	double memory = 1.0;
	while (cnt <= loop) {
		sum = sum + 1 / (cnt * calc_powInte(x, cnt));
		printf("lnB: cnt: %d, sum: %.15Lf, abs: %.15Lf\n", cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		memory = sum;
		cnt++;
	}
	return sum;
}

double calc_ln_test(double x, int loop) {
	if (x >= 0.5 && x <= 1.5) { //lnA에 x-1 대입
		return calc_lnA(x - 1, loop);
	}
	else if (x > 0 && x < 0.5) { //lnB에 대입 후 lnA 이용
		return -calc_lnB(x + 1, loop) + calc_lnA(x, loop);
	}
	else if (x > 1.5 && x < 2.0) { //lnB에 대입 후 lnA 이용
		return calc_lnB(x, loop) + calc_lnA(x - 2, loop);
	}
	else { //lnA에 1/1+x - 1 대입 후  - 붙임
		double x0 = x;
		int cnt = 0;
		while (x0 >= 2) {
			x0 = calc_root(x0);
			cnt++;
		}
		printf("return! : %.15Lf, cnt: %d\n", 1 / x0, cnt);
		return -calc_ln_test(1/x0, loop)*calc_powInte(2,cnt);
	}
}
double calc_ln(double x) {
	double x0;
	if (x >= 0.5 && x <= 1.5) { //lnA에 x-1 대입
		return calc_lnA(x - 1);
	}
	else if (x > 2) { //lnB에 대입 후 lnA 이용
		x0 = x;
	}
	else { //lnA에 1/1+x - 1 대입 후  - 붙임
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

double calc_sinA(double x) { //-pi에서 +pi까지 입력 받을 함수
	int cnt = 0;
	double sum = 0.0;
	double memory = 1.0;
	while (1) {
		sum = sum + calc_powInte(-1, cnt) * calc_powInte(x, 2 * cnt + 1) / calc_facto(2 * cnt + 1);
		printf("SinA: cnt: %d, sum: %.15Lf, abs: %.15Lf\n", cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		memory = sum;
		cnt++;
	}
	return sum;
}

double calc_sinMakeA(double x) {
	double n = 0;
	if (x < pi && x > -pi) {
		return x;
	}
	else if (x >= pi) {
		printf("+.");
		while (x >= n) { //n이 x와 같아지거나 x보다 커지기 전까지 반복
			n += pi;
			printf("n: %.14Lf, x: %.15Lf\n", n, x);
		}; // n - x = pi - A
		return pi - n + x;
	}
	else {
		printf("-.");
		while (x <= n) { //n이 x와 같아지거나 x보다 작아지기 전까지 반복
			n -= pi;
			printf("n: %.14Lf, x: %.15Lf\n", n, x);
		} // n - x = pi - A
		return -pi + x - n;
	}
}

/* 수정중
double calc_sin(double x) { //x를 sinA의 범위 안으로 변환, 입력, 출력
	double a = calc_sinMakeA(x);
	int index = 0;
	if (a < 0) {
		index = 1;
		a = -a;
	}
	if (a >= 0 && a <= pi / 2) {
		return calc_sinA(a);
	}
	else if (a > pi/2 && a <= pi) {

	}
	else if (a > pi && a <= pi*3/2) {

	}
	else { // a > pi*3/2 && s<= 2*pi

	}
}
*/

void main() {
	printf("MakeA: %.15Lf", calc_sinMakeA(-4*pi-pi/2));
	//printf("ln0.1: %.15Lf, facto: %d, powInte: %d\n", calc_ln(0.0000000001), facto, powInte);
}
