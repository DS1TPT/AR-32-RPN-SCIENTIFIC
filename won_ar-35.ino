// 원종완이 쓴 코드는 여기에...
// 테스트 중... 아직 쓸게 못됨.
#include <stdio.h>

double abs(double x) { //절댓값 구하는 함수
	if (x > 0) {
		return x;
	}
	else {
		return -x;
	}
}

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

double pow(double x, double y) { //power를 정수 범위에서 실행
	double n = x;                //y에 실수 범위 지정시 오류
	if (y == 0) {                //밑에 쓰인 함수들이 대부분 이 함수를 쓰므로 실수 계산에 있어 제약이 있음.
		n = 1;                   //빠른 시일내로 실수가 가능하도록 변경할 필요가 있음.
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

double root(double x, int accuracy) { //정수 범위 내에서 바빌로니아 법을 이용해서 x^(1/2) 실행
	double n = x / 2;                 //accuracy는 허용할 오차 ex: 0.0000000001
	while (1) {           //powInte함수를 사용하는 함수임.
		double memory = n;
		if (pow(n, 2) == x) {
			break;
		}
		else {
			n = (pow(n, 2) + x) / (2 * n);
			/*printf("root running now n= %Lf, memory= %Lf, accuracy= %d, bool= %d\n" //테스트용 출력항
				, n, memory, accuracy, abs(memory - n) < 0.00001);*/
			if (abs(memory - n) < accuracy) {
				break;
			}
		}
	}
	return n;
}

double rootInte(double a, double b, int accuracy) { //root를 보다 일반화한 a^(1/b)를 구하는 함수
	double n = a;                                   //이 역시 powInte를 쓰고 accuracy에 대한 내용 역시 root와 같다.
	while (1) {
		double memory = n;
		n = n - ((pow(n, b) - a) / (b * pow(n, b - 1)));//뉴튼 랩튼법
		//printf("test rootInte= %f, dy/dx= %f, accuracy= %d\n", n, (b * powInte(n, b - 1)), accuracy); //테스트용 출력항
		if (abs(memory - n) < accuracy) {
			break;
		}
	}
	return n;
}

double testF(double x) {
	return pow(x-2, 2); //(x-2)^2
}

double dydxTestF(double x) {
	return 2 * x - 4;
}

double testNewton(double(*fx)(double x), double(*dydx)(double x), double startNumber ,int accuracy) {
	double n = startNumber;                                  
	while (1) {
		double memory = n;
		n = n - fx(n)/dydx(n);//뉴튼 랩튼법
		//printf("test n= %f, memory= %f, f(x)= %f, dy/dx= %f, accuracy= %d\n", n, memory, fx(n), dydx(n), accuracy); //테스트용 출력항
		if (abs(memory - n) < accuracy) {
			break;
		}
	}
	return n;
}

double gammaF(double z) { //항간의 차이가 0.000001가 날때까지 15000번 연산이 필요함. 
	double result = 1/z;  //사실상 폐기
	double n = 1;         //facto에 x를 넣을때 여기에는 x+1를 넣어야함을 주의
	int count = 0;
	double memory = 0;
	while (1){
		result = result * pow(1 + (1 / n), z) / (1 + (z / n));
		//printf("*test result= %f, memory= %f, count= %d\n",result, memory, count);
		n += 1;
		if (abs(memory - result) < 0.000001) {
			break;
		}
		else {
			count += 1;
		}
		memory = result;
	}
	return result;
}

double facto(double x) {
	//0보다 작은 수 혹은 실수가 들어올 경우 에러
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

double sin(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1){
		if (abs(memory - sum) < 0.0000000000000000000000000001) {
			break;
		}
		memory = sum;
		u = pow(-1, i) * pow(x, 2 * i + 1) / facto(2 * i + 1);
		//printf("sum= %f30, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

double cos(double x) {
	double sum = 0;
	double u = 0;
	double i = 0;
	double memory = 1;
	while (1){
		if (abs(memory - sum) < 0.0000000000000000000000000001) {
			break;
		}
		memory = sum;
		u = pow(-1, i) * pow(x, 2 * i) / facto(2 * i);
		//printf("sum= %f30, u= %f i= %f\n", sum, u, i);
		sum = sum + u;
		i++;
	}
	return sum;
}

double tan(double x) {
	return sin(x) / cos(x); //계산량이 부담된다면 개선 가능
}

void main() {
	/*printf("%f\n", powInte(2, 3));
	printf("%f\n", powInte(2, 0));
	printf("%f\n", powInte(2, -3));
	printf("%f\n", root(6, 100));
	printf("%f\n", rootInte(6, 50, 100));
	printf("%f\n", testNewton(testF, dydxTestF,-30 ,100));*/
	printf("sin= %f, cos= %f\n", sin(-2), cos(-2));
	printf("gamma= %f, facto= %f\n", gammaF(5), facto(4));
}
