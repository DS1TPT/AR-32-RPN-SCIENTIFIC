#include <stdio.h>

const double pi = 3.14159265359;
const double e = 2.71828182846;

//accuracy
const double acc = 0.00000000000001;
const double loop = 1000;

//연산량 확인용 전역변수
int facto = 0;
int powInte = 0;
//abs
//절댓값
double calc_abs(double x) {
	if (x > 0) {
		return x;
	}
	else {
		return -x;
	}
}
//x!
//x>=0
double calc_facto(double x) {
	if (x == 0) {
		return 1;
		//facto++; 연산량 확인용
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
// x^y
// x >= 0  -inf<y(정수)<+inf
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
//x^(1/2)
//x>=0
double calc_root(double x) {
	double n = x / 2;// 개선 좀 하고 싶음
	double m0;
	int cnt = 0;
	double outMemory = 0.0;
	while (1) {
		double memory = n;
		if (cnt % 2 == 1) {
			//printf("true 1 : cnt : %d\n", cnt);
			if (outMemory == n) {
				//printf("true 2 : outMemory: %.15Lf \n", outMemory);
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
			//printf("root running now n= %.15Lf, memory= %.15Lf, outMemory: %.15Lf, bool= %d\n" //테스트용 출력항
				//, n, memory, outMemory, calc_abs(memory - n) < acc);
			if (calc_abs(memory - n) < acc) {
				break;
			}
			cnt++;
		}
	}
	return n;
}
//ln함수의 부속품
//메클로린 연산 파트
double calc_lnA(double x) { //return ln(x+1)
	int cnt = 1;
	double sum = 0.0;
	while (1) {
		double memory = sum;
		sum = sum + calc_powInte(-1.0, cnt + 1) * calc_powInte(x, cnt) / cnt;
		//printf("lnA: cnt: %d, sum: %.15Lf, abs: %.15Lf\n", cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		cnt++;
	}
	return sum;
}
//lnx
//x>0
double calc_ln(double x) {
	double x0;
	if (x >= 0.5 && x <= 1.5) { //lnA에 x-1 대입
		return calc_lnA(x - 1);
	}
	else if (x > 2) { //lnB에 대입 후 lnA 이용
		x0 = x;
	}
	else { //lnA에 1/1+x - 1 대입 후  - 붙임
		x0 = 1 / x;
	}
	int cnt = 0;
	while (x0 >= 2) {
		x0 = calc_root(x0);
		cnt++;
	}
	if (x > 2) {
		//printf("return! : %.15Lf, cnt: %d\n", 1 / x0, cnt);
		return -calc_ln(1 / x0) * calc_powInte(2, cnt);
	}
	else {
		//printf("turn! : %.15Lf, cnt: %d\n", 1 / x0, cnt);
		return calc_ln(1 / x0) * calc_powInte(2, cnt);
	}
}
//sin함수의 부속품
double calc_sinA(double x) { //-pi에서 +pi까지 입력 받을 함수
	int cnt = 0;
	double sum = 0.0;
	while (1) {
		double memory = sum;
		sum = sum + calc_powInte(-1, cnt) * calc_powInte(x, 2 * cnt + 1) / calc_facto(2 * cnt + 1);
		//printf("SinA: x: %.15Lf, cnt: %d, sum: %.15Lf, abs: %.15Lf\n", x, cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		cnt++;
	}
	return sum;
}
//실수 범위 모듈러 연산
//sin연산에 쓰임
double calc_mod(double x, double y) {
	double x0 = x / y;
	x0 = x0 - (int)x0;
	//printf("modToSin: %.15Lf\n", x0 * y );
	return x0 * y;
}
//sinx
//-inf<x<+inf
double calc_sin(double x) { //x를 sinA의 유효범위 안으로 변환, 입력, 출력
	double a = calc_mod(x, 2 * pi);
	int index = 1;
	if (a < 0) {
		index = -1;
		a = -a;
	}
	if (a >= 0 && a <= pi / 2) {
		return index * calc_sinA(a);
	}
	else if (a > pi / 2 && a <= pi) {
		return index * calc_sinA(pi - a);
	}
	else if (a > pi && a <= pi * 3 / 2) {
		return (-1) * index * calc_sinA(a - pi);
	}
	else { // a > pi*3/2 && s<= 2*pi
		return (-1) * index * calc_sinA(2 * pi - a);
	}
}
//cosx
//-inf<x<+inf
//sinx에 의존
double calc_cos(double x) {
	return calc_sin(x + (pi / 2));
}
//tanx
//-inf<x<+inf (2pi*n + pi/2에서 +inf로 발산, 2pi*n + pi*3/2에서 -inf로 발산) 
//sinx, cosx에 의존
//발산 범위가 입력될 시 오류처리에 신경써야함.
double calc_tan(double x) {
	return calc_sin(x) / calc_cos(x);
}
//e^x
//-inf<x<+inf
//도출되는 값이 표시범위 안쪽이면 출력함.
//도출되는 값이 표시범위를 넘은 것에 대한 오류 처리가 필요
double calc_exp(double x) {
	int cnt = 0;
	double sum = 0.0;
	double u = calc_powInte(e, (int)x);
	printf("u: %.15Lf\n", u);
	while (1) {
		double memory = sum;
		sum = sum + (u / calc_facto(cnt)) * calc_powInte(x - (int)x, cnt);
		printf("e^x: x: %.15Lf\n, cnt: %d\n, sum: %.15Lf\n, abs: %.15Lf\n", x, cnt, sum, calc_abs(memory - sum));
		if (calc_abs(memory - sum) < acc) break;
		cnt++;
	}
	return sum;
}
//x^y
//x>=0 , -inf<y<+inf
//exp, lnx 사용
double calc_pow(double x, double y) {
	return calc_exp(y * calc_ln(x));
}
//arcsinx
// -1<x<1
//정확도 이슈가 있음
//이론적으로 정확해지면 따로 말함
double calc_arcsin(double x) {
	double sum = 0.0;
	int cnt = 0;
	double outMemory = 0.0;
	while (1) {
		double memory = sum;
		if (cnt % 2 ==1) {
			if (outMemory == memory){
				break;
			}
			outMemory = memory;
		}
		sum = sum - (calc_sin(sum) - x) / calc_cos(sum);
		if (calc_abs(memory - sum) < acc) break;
		printf("cnt: %d, sum: %.15Lf, memory: %.15Lf\n", cnt, sum, memory);
		cnt++;
	}
	return sum;
}
//arccosx
// -1<x<1
//arcsinx에 의존
double calc_arccos(double x) {
	return pi / 2 - calc_arcsin(x);
}
//arctanx
// -inf<x<+inf
//arcsinx에 의존
double calc_arctan(double x) {
	double index = 1;
	if (x < 0) index = -1;
	double t = (1 + (1 / calc_powInte(x, 2)));
	return index*calc_arcsin(calc_root(1 / t));
}

void main() {
	double x = -12312415;
	printf("arctan(%.1lf): %.15Lf, facto: %d, powInte: %d\n", x, calc_arctan(x), facto, powInte);
}
