/* 구현할 때 요구사항 및 주의사항
*  1. 0으로 나누기와 같은 건 명령어가 들어왔을 때 오류를 띄우고 막아야 함.
*  -> 오류는 위 왼쪽에서 ERROR만 띄우는 걸로 충분함.
*  2. RPN으로 구현함.
*  3. 제곱(SQ)은 X 레지스터의 값의 Y 제곱으로 구현함
*  4. 입력은 11자리까지만 받을 것(부호1 + 가수12자(.포함) + 지수3자리 + 널 = 17글자).
*  -> -1.3456789012-10 NULL과 같이 넣어짐 
*  5. 입력은 처리되기 전까진 버퍼에 저장됨(처리 전까지 X 레지스터는 임시 변수로 쓰임)
*  5-1 결과값은 결과 출력 후 키 입력 전까지는 X 레지스터에 들어가고, 키 입력 후 Y로 올라감.
*  -> 500 엔터 20 누르면 Y에 500, X는 20임.
*  6. math.h 써도 됨. 단, 이걸 쓴다면 "상수 정의"에서 pi 상수는 주석처리할 것.
*/

//#pragma GCC optimize ("-O0")
//#pragma GCC push_options

#define _DEF_PRECISION_FLOAT32

#include <fp64lib.h> // 64비트 부동소수점
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <stdlib.h>
#include <string.h>

// 버튼 명령별로 쓸 문자 지정
#define BTN_PWR 1
#define BTN_LOG 2
#define BTN_LN 3
#define BTN_EX 4
#define BTN_CLR 5
#define BTN_SQRT 6
#define BTN_SHIFT 7
#define BTN_SIN 8
#define BTN_COS 9
#define BTN_TAN 10
#define BTN_RECIPROCAL 12
#define BTN_EXCHANGEXY 13
#define BTN_CHS 14
#define BTN_EEX 15
#define BTN_CLX 16
#define BTN_ENTER 17

#define LEFT 0 // shiftBuffer 함수에서 방향을 지정할 때 쓰임
#define RIGHT 1

// 모드 정의
#define MODE_RES 0 // 결과 표시
#define MODE_IN 1 // 입력중 표시
#define MODE_BUSY 2 // 계산중 표시
#define MODE_ERR 3 // 오류 표시

// 오류 정의
#define NO_ERR 0 // 오류 없음
#define ERR_OOR 1 // OUT OF RANGE 오류
#define ERR_DIVZERO 2 // 0으로 나누기 오류
#define ERR_MATH 3 // 범위 초과를 제외한 수학적 오류
// 다른 오류도 필요하면 정의할 것

// 기타 정의
#define ANGLE_DEG 0
#define ANGLE_RAD 1
#define ANGLE_GRAD 2

#ifdef _DEF_PRECISION_FLOAT32
#define ACCURACY 0.000001
#else
#define ACCURACY 0.00000000000001
#endif

// 상수 정의
const double pi = 3.141592653589793238; // math.h 쓰면 주석처리할 것
const byte ROWS = 4; // 행 버튼 개수
const byte COLS = 4; // 열 버튼 개수

const char keysD[ROWS][COLS] = { // 윗 키패드
    {'1','2','3','-'},
    {'4','5','6','+'},
    {'7','8','9','x'},
    {'0','.','p','/'}
};
const char keysU[ROWS][COLS] = { // 아래 키패드
    {BTN_LOG, BTN_LN, BTN_EX, BTN_CLR},
    {BTN_SHIFT, BTN_SIN, BTN_COS, BTN_TAN},
    {BTN_PWR, BTN_SQRT, BTN_RECIPROCAL, BTN_EXCHANGEXY},
    {BTN_ENTER, BTN_CHS, BTN_EEX, BTN_CLX}
};
const byte rowPinsD[ROWS] = { 6, 7, 8, 9 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsD[COLS] = { 2, 3, 4, 5 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 
const byte rowPinsU[ROWS] = { A0, A1, A2, A3 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsU[COLS] = { 10, 11, 12, 13 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 


// 전역변수 목록
volatile double regX, regY, regZ, regT; // 레지스터 XYZT, 수시로 값이 바뀔 수 있어 최적화 제외
bool isShift = false; // Shift 상태변수
char buffer[17] = { 0, }; // 입력 버퍼(문자열)
char op = 0; // 연산자를 저장하는 변수
byte errCode = NO_ERR; // 오류 코드를 임시로 저장하는 변수
byte angleMode = ANGLE_RAD;
#ifdef _DEF_PRECISION_FLOAT32 
bool is64b = false; // 64비트 부동소수점을 쓰는지 저장
#else
const bool is64b = true;
#endif
bool is bklight = true; // 백라이트 켜는지 끄는지 저장

// 키패드 라이브러리 설정 부분
Keypad kpdU = Keypad(makeKeymap(keysU), rowPinsU, colPinsU, ROWS, COLS);
Keypad kpdD = Keypad(makeKeymap(keysD), rowPinsD, colPinsD, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD. 작동이 되지 않으면 주소를 0x27로 해볼 것


// 프로그램 시작점
void setup() {
    lcd.init(); //LCD 시작
    lcd.backlight();
    regX = 0.0;
    regY = 0.0;
    regZ = 0.0;
    regT = 0.0;
}

void loop() {
    char keyD = kpdD.getKey(); // 아래 키패드 입력받은 것을 keyD에 저장
    char keyU = kpdU.getKey(); // 윗 키패드 입력받은 것을 keyU에 저장
    if (keyD) { // 아래 키패드 처리
        switch (keyD) {
            case '1':

            break;

            case '2':

            break;

            case '3':

            break;

            case '4':

            break;

            case '5':

            break;

            case '6':

            break;

            case '7':

            break;

            case '8':

            break;

            case '9':

            break;

            case '0':

            break;

            case '.':

            break;

            case 'p':
            if (regX != 0.0) {
                
            }
            break;

            case '-':
            regX = regY - regX;
            rollDownReg();
            break;

            case '+':
            regX = regY + regX;
            rollDownReg();
            break;

            case 'x':
            regX = regY * regX;
            rollDownReg();
            break;

            case '/':
            bufferToRegX(true);
            if (regX == 0.0) goto loop_err;
            regX = regY / regX;
            rollDownReg();
            break;
        }
        proc(); // 아래 키패드가 눌렸다면 처리 함수 호출
    } 
    else if (keyU) { // 윗 키패드 처리
        switch (keyU) {
            case BTN_LOG:

            break;

            case BTN_LN:

            break;

            case BTN_EX:

            break;

            case BTN_CLR:
            clearMem();
            break;

            case BTN_ARC:
            if (!isArc) isArc = true;
            else isArc = false;
            break;

            case BTN_SIN:

            break;

            case BTN_COS:

            break;

            case BTN_TAN:

            break;

            case BTN_PWR:

            break;

            case BTN_SQRT:

            break;

            case BTN_RECIPROCAL:

            break;

            case BTN_EXCHANGEXY:

            break;

            case BTN_ENTER:

            break;

            case BTN_CHS:
            bufferToRegX(false);
            if (regX > 0.0) {
                shiftBuffer(RIGHT);
                buffer[0] = '-';
            }
            else if (regX < 0.0) {
                shiftBuffer(LEFT);
            }
            break;

            case BTN_EEX:

            break;

            case BTN_CLX:
            memset(buffer, 0, sizeof(buffer));
            regX = 0.0;
            break;
        }
        proc(); // 윗 키패드가 눌렸다면 처리 함수 호출
    }
    return;

    loop_err: // 루프 함수에서 생긴 오류 처리
    if (regX == 0.0 && op == '/') { // 0으로 나누기 오류
        errCode = ERR_DIVZERO;
    }
    printLCD(MODE_ERR);
}

void proc() { // 처리 함수
    if (op) { // 연산자 입력 받음
        printLCD(MODE_BUSY); // 계산중이라고 띄워놓고 연산 시작
        // 연산을 처리하는 부분을 아래에 적기
    }
    else { // 연산자 입력 없음
        printLCD(MODE_IN); // 입력 출력
        return; // 함수 종료
    }
    printLCD(MODE_RES); // 결과 출력
    return;

    proc_err: // 처리 함수에서 생긴 오류 처리
    if (regX >= 1.0e+100 || regX <= 1.0e-100) { // 범위 초과 오류
        errCode = ERR_OOR;
    }
    // 다른 오류 처리 코드 넣기

    printLCD(MODE_ERR); // 오류 표시
}

void rollDownReg() { // 레지스터 하나씩 내리는 함수, Y->X는 구현 안함(의도됨)
        regY = regZ;
        regZ = regT;
        regT = 0;
}

void printLCD(byte mode) {
    char str[16] = { 0, };
    lcd.clear();
    lcd.setCursor(0, 0); // 윗줄 처음으로 커서 설정
    if (mode == MODE_ERR) { // 오류 표시
        if (errCode == ERR_OOR) {
            lcd.print("?OUT OF RANGE");
        }
        else if (errCode == ERR_DIVZERO) {
            lcd.print("?DIVIDE BY 0");
        }
        else if (errCode == ERR_MATH) {
            lcd.print("?MATH ERROR");
        }
        // 다른 오류 코드 표시 부분 구현하기

        lcd.setCursor(0, 1); //PRESS CLX 표시
        lcd.print("PRESS CLX");
        errWait(); // 오류 입력 대기
        return; // 함수 종료
    }
    if (mode == MODE_BUSY) { // 계산중 표시
        lcd.print("BUSY!");
        return;
    }
    if (mode == MODE_RES) { // 결과값을 표시하는 부분
        dtostrf(regX, -16, 12, str);
        lcd.print(str);
    }
    else if (mode == MODE_IN) { // 입력값을 표시하는 부분
        lcd.print(buffer);
    }
    // 상태 정보를 아랫줄에 표시
    lcd.setCursor(0, 1); // 아랫줄 처음으로 커서 설정
    if (
    if (isShift) { // arc 눌렀으면 표시
        lcd.setCursor(4, 1);
        lcd.print("arc");
    }
    if (mode == MODE_RES) { // 결과값 표시면 RESULT라고 표시
        lcd.setCursor(11, 1);
        lcd.print("RESULT");
    }
    else if (mode == MODE_IN) { // 입력중이면 INPUT 표시
        lcd.setCursor(12, 1);
        lcd.print("INPUT");
    }
}

void errWait() {
    char keytmp = 0;
    while(1) {
        keytmp = kpdU.getKey();
        if (keytmp == BTN_CLX) break;
    }
    delay(500); // 0.5초 기다림
    regX = 0.0; // X 레지스터의 값을 0으로 초기화
    memset(buffer, 0, sizeof(buffer)); // 입력을 모두 지움
    op = 0; // 연산자 지움
}

/* 기능 함수 구현 */
void shiftBuffer(byte dir) { // 버퍼에서 문자를 한 방향으로 미는 함수, 순환 없음
    int len = sizeof(buffer); // 버퍼 크기 저장
    if (dir == RIGHT) {
        for (int i = len - 2; i >= 0; i--) {
            buffer[i + 1] = buffer[i];
        }
    }
    else if (dir == LEFT) {
        for (int i = 0; i < len - 1; i++) {
            buffer[i] = buffer[i + 1];
        }
    }
    buffer[17] = 0; // 버퍼 마지막은 반드시 null이 들어감
}

void bufferToRegX(bool clrBuffer) { // 버퍼의 값을 레지스터 X로 복사.
    char* eptr;
    regX = strtod(buffer, &eptr);
    if (clrBuffer) memset(buffer, 0, sizeof(buffer)); // 인수가 참일 때만 버퍼 지움
}

void clearMem() { // 메모리 비우는 함수
    memset(buffer, 0, sizeof(buffer));
    regX = 0.0;
    regY = 0.0;
    regZ = 0.0;
    regT = 0.0;
    isArc = false;
    op = 0;
}

char* szParse(char* sz, const char* delim) { // 문자열의 특정 부분의 주소를 반환하는 함수(parse)
    char* pDelimCh;
    while (*sz) {
        pDelimCh = (char*)delim;
        while (*pDelimCh) {
            if (*sz == *pDelimCh++) *sz = NULL;
        }
        if (!*sz) return ++sz;
        sz++;
    }
    return NULL;
}

void szAppend(char *sz, const char* ch) { // 글자를 문자열에 덧붙이는 함수
    char* ptr = sz;
    for (int i = 0; i < 11; i++) {
      if (*ptr = 0) {
        *ptr++ = ch;
        *ptr = 0;
      }
      ptr++;
    }
}


/* 수학 함수 구현 */

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

//#pragma GCC pop_options
