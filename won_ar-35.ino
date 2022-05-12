// 원종완이 쓴 코드는 여기에...
// 테스트 중... 아직 쓸게 못됨.
#include <stdio.h>
#define ACCURACY 0.00000001
/*ACCURACY를 10 ^ (-8)로 한 이유:
window에서는 double이 소수점 6자리까지 정확도를 보장하기에
반올림까지 생각하여 10^(-7) ~ 10^(-8)이 적절해 보였기 때문
아두이노 환경에서 보장하는 소수점 자리 수가 바뀐다면 바꿀 필요가 있다.*/

//x의 절댓값을 내보내는 함수
double abs(double x) { 
	if (x > 0) {
		return x;
	}
	else {
		return -x;
	}
}

/*window double은 소수점 6자리까지만 보장하기 때문에 하드코딩함. 
혹시 아두이노 환경에서 필요하다면 쓸 수도 있는 함수*/
double numberingDouble(double target) {
	double underPoint = target - (int)target;
	char underPointChar[100];
	double cnt = 0;
	sprintf_s(underPointChar, sizeof(underPointChar), "%f", underPoint);
	//while (testChar[cnt++] != '.');
	while (1) {
		if (underPointChar[(int)cnt] == NULL) {
			break;
		}
		else {
			cnt++;
		}
		printf("test: %c\n", underPointChar[(int)cnt]);
	}
	return cnt - 2;
}

/* x^y 값을 내보내는 함수
x는 실수 범위 y는 정수 범위*/
double powInte(double x, double y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
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

/*a ^ (1 / b)를 출력하는 함수
a는 실수 범위, b는 정수 범위*/
double rootInte(double a, double b) {
	double n = 1;
	while (1) {
		double memory = n;
		n = n - ((powInte(n, b) - a) / (b * powInte(n, b - 1)));//뉴튼 랩튼법
		//printf("test rootInte= %f, dy/dx= %f, accuracy= %f\n", n, (b * powInte(n, b - 1)), ACCURACY); //테스트용 출력항
		if (abs(memory - n) < ACCURACY) {
			break;
		}
	}
	return n;
}

/*x^y를 출력하는 함수
x, y 모두 실수 범위*/
double pow(double x, double y) { //power를 정수 범위에서 실행
	double yD = 1000000;
	double yU = y * yD;
	//printf("yU: %f, yD: %f\n", yU, yD);
	double root = rootInte(x, yD, ACCURACY);
	double result = powInte(root, yU);
	return result;
}

//x^(1/2)를 출력하는 함수
//x는 실수 범위
//바빌로니아법 이용
double root(double x) { 
	double n = x / 2;                 
	while (1) {           
		double memory = n;
		if (powInte(n, 2) == x) {
			break;
		}
		else {
			n = (powInte(n, 2) + x) / (2 * n);
			/*printf("root running now n= %Lf, memory= %Lf, bool= %d\n" //테스트용 출력항
				, n, memory, abs(memory - n) < 0.00001);*/
			if (abs(memory - n) < ACCURACY) {
				break;
			}
		}
	}
	return n;
}

//함수 testNewton의 작동을 확인하기 위한 함수
double testF(double x) {
	return powInte(x-2, 2); //(x-2)^2
}

double dydxTestF(double x) {
	return 2 * x - 4;
}

//뉴튼-랩튼법을 모듈화시킨 것
//전달받은 함수와 해당 함수의 미분계수로 뉴튼-랩튼법을 시행한다.
//시작값에 따라 연산량이 줄 수 있다.
double testNewton(double(*fx)(double x), double(*dydx)(double x), double startNumber) {
	double n = startNumber;                                  
	while (1) {
		double memory = n;
		n = n - fx(n)/dydx(n);//뉴튼 랩튼법
		//printf("test n= %f, memory= %f, f(x)= %f, dy/dx= %f", n, memory, fx(n), dydx(n)); //테스트용 출력항
		if (abs(memory - n) < ACCURACY) {
			break;
		}
	}
	return n;
}

/*실수 범위에서 (z-1)!과 같은 역할을 하는 함수
필요 연산량에 비해 오차가 큼*/
double gammaF(double z) {  
	double result = 1/z;  
	double n = 1;        
	int count = 0;
	double memory = 0;
	while (1){
		result = result * powInte(1 + (1 / n), z) / (1 + (z / n));
		//printf("*test result= %f, memory= %f, count= %d\n",result, memory, count);
		n += 1;
		if (abs(memory - result) < ACCURACY) {
			break;
		}
		else {
			count += 1;
		}
		memory = result;
	}
	return result;
}

//x!을 출력하는 함수
//x는 0보다 크거나 같은 정수 범위
double facto(double x) {
	if (x == 0) {
		return 1;
	}
	else {
		double sum = 1;
		for (double i = 2; i <= x; i++) {
			sum = sum * i;
		}
		return sum ;
	}
}

//sinx를 출력하는 함수
//x는 실수 범위
double sin(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1){
		if (abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = powInte(-1, i) * powInte(x, 2 * i + 1) / facto(2 * i + 1);
		//printf("sum= %f30, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//cosx를 출력하는 함수
//x는 실수 범위
double cos(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1){
		if (abs(memory - sum) < ACCURACY) {
			break;
		}
		memory = sum;
		u = powInte(-1, i) * powInte(x, 2 * i) / facto(2 * i);
		printf("sum= %f30, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

//tanx를 출력하는 함수
//x는 실수 범위
//계산량이 부담된다면 개선 가능
double tan(double x) {
	return sin(x) / cos(x); 
}

//소수점 6자리 이하로 내려가는 숫자의 입력 대해 필터링을 할 필요가 있어보임

void main() {
	printf("sin(2)= %f, rootInte= %f, pow= %f, numberingDouble= %f"
		, sin(2), rootInte(1.24, 100000), pow(1.2, 1.243), numberingDouble(3123.245564));
}
