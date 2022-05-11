// 원종완이 쓴 코드는 여기에...
// 테스트 중... 아직 쓸게 못됨.
#include <stdio.h>

double abs(double x) {
	if (x > 0) {
		return x;
	}
	else {
		return -x;
	}
}

double powInte(double x, double y) {
	double n = x;
	if (y == 0) {
		n = 0;
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

double root(double x, int accuracy) {
	double n = x/2;
	while (accuracy != 0) {
		double memory = n;
		if (powInte(n, 2) == x) {
			break;
		}
		else {
			n = (powInte(n, 2) + x) / (2 * n);
			accuracy--;
			printf("root running now n= %Lf, memory= %Lf, accuracy= %d, bool= %d\n"
             , n, memory, accuracy, abs(memory - n) < 0.00001);
			if(abs(memory - n)<0.00001) {
				break;
			}
		}
	}
	return n;
}

double rootInte(double a, double b, int accuracy) {
	double n = a;
	while (accuracy != 0) {
		double memory = n;
		n = n - ((powInte(n, b) - a) / (b * powInte(n, b - 1)));
		printf("test rootInte= %f, dy/dx= %f, accuracy= %d\n", n, (b * powInte(n, b - 1)), accuracy);
		if (abs(memory - n) < 0.00001) {
			break;
		}
		accuracy--;
	}
	return n;
}

void main(){
	printf("%f\n", powInte(2, 3));
	printf("%f\n", powInte(2, 0));
	printf("%f\n", powInte(2, -3));
	printf("%f\n", root(6, 100));
	printf("%f\n", rootInte(6, 50, 100));
	
	long long int i, a, *ap;

	double fd = -0.01;

	ap = (long long int*)&fd;

	a = *ap;

	for (i = 63; i >= 0; i--){
		printf("%d", a >> i & 1);
	}
}
