#include <stdio.h>
#include <stdlib.h>

/*변수로 크기 sizeof(char)*3의 배열을 받아서 결과값을 해당 문자열로 내보내는 함수
stdlib.h에 의존성 없음. 
근데 나는 main에서 출력할때 어떻게 비벼도 stdlib의 malloc로 포인터 초기화해야 돌아가더라.
그러므로 걍 아래있는 readAfterE_withLib_ 쓰는 거 추천
만약에 E+99를 넘어서 E+100이 나오면 오류가 아닌 "E+100"(char*4짜리)를 출력함*/
void readAfterE_(double target, char str[]) {
	int cnt = 0;
	int cntI = 0;
	char beforePointChar[100];
	char caseZero[4] = "_00";
	//printf("test1: %e\n", target);

	sprintf_s(beforePointChar, sizeof(beforePointChar), "%e", target);

	while (1) {
		if (beforePointChar[cnt] == 'e') {
			if (beforePointChar[cnt + 3] != '0') {
				while (beforePointChar[cnt + cntI+1] != NULL) {
					str[cntI] = beforePointChar[cnt + cntI + 1];
					//printf("test2.1: %c\n", str[cntI]);
					cntI++;
				}
			}
			else {
				for (int i = 0; i < 3; i++) {
					str[i] = caseZero[i];
					//printf("test2.2: %c\n", str[i]);
				}
			}
			break;
		}
		else {
			cnt++;
		}
	}
}

/*stdlib의 malloc를 사용해서 함수 내에서 메모리 할당을 하는 버전의 함수
더이상 변수로 배열을 받을 필요없이 직접 char*형식으로 내보낸다.*/
char* readAfterE_withLib(double target) {
	int cnt = 0;
	int cntI = 0;
	char* returnChar = malloc(sizeof(char) * 3);
	char beforePointChar[100];
	char caseZero[4] = "_00";
	//printf("test1: %e\n", target);

	sprintf_s(beforePointChar, sizeof(beforePointChar), "%e", target);

	while (1) {
		if (beforePointChar[cnt] == 'e') {
			if (beforePointChar[cnt + 3] != '0') {
				while (beforePointChar[cnt + cntI + 1] != NULL) {
					returnChar[cntI] = beforePointChar[cnt + cntI + 1];
					//printf("test2.1: %c\n", returnChar[cntI]);
					cntI++;
				}
			}
			else {
				for (int i = 0; i < 3; i++) {
					returnChar[i] = caseZero[i];
					//printf("test2.2: %c\n", returnChar[i]);
				}
			}
			break;
		}
		else {
			cnt++;
		}
	}
	return returnChar;
}

void main() {
	char* test = malloc(sizeof(char)*3);

	double testD = 0.0;
  
  printf("E의 뒤가 보고 싶은 건가 자네?\n어떤 E의 뒷태가 보고 싶은 거지?(적당한 소수 입력) : ");

	scanf_s("%Lf", &testD, sizeof(testD));
	//readAfterE_(testD, test);
	test = readAfterE_withLib(testD);

	for (int i = 0; i < 3; i++)
	{
		printf("test3 c: %c\n", test[i]);
	}
}
