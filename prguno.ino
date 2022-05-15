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
#define BTN_ROLLDOWN 5
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

// 상수 정의
const float64_t ACCURACY = (float64_t)0.00000000000001
const float64_t pi = 3.141592653589793238; // math.h 쓰면 주석처리할 것
const byte ROWS = 4; // 행 버튼 개수
const byte COLS = 4; // 열 버튼 개수

const char keysD[ROWS][COLS] = { // 윗 키패드
    {'1','2','3','-'},
    {'4','5','6','+'},
    {'7','8','9','x'},
    {'0','.','p','/'}
};
const char keysU[ROWS][COLS] = { // 아래 키패드
    {BTN_LOG, BTN_LN, BTN_EX, BTN_ROLLDOWN},
    {BTN_SHIFT, BTN_SIN, BTN_COS, BTN_TAN},
    {BTN_PWR, BTN_SQRT, BTN_RECIPROCAL, BTN_EXCHANGEXY},
    {BTN_ENTER, BTN_CHS, BTN_EEX, BTN_CLX}
};
const byte rowPinsD[ROWS] = { 6, 7, 8, 9 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsD[COLS] = { 2, 3, 4, 5 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 
const byte rowPinsU[ROWS] = { A0, A1, A2, A3 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsU[COLS] = { 10, 11, 12, 13 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 


// 전역변수 목록
volatile float64_t regX, regY, regZ, regT; // 레지스터 XYZT, 수시로 값이 바뀔 수 있어 최적화 제외
char buffer[12] = { 0, }; // 입력 버퍼(문자열)
char expBuf[4] = { 0, }; // 지수 입력 버퍼(문자열)
char op = 0; // 연산자 저장
byte errCode = NO_ERR; // 오류 코드 임시 저장
byte angleMode = ANGLE_RAD;
bool isShift = false; // Shift 상태 변수
bool isBkLight = true; // 백라이트 켜는지 끄는지 저장
bool isBlockInput = false; // 입력을 막는지 지정
bool isDecimal = false; // 소수점 있는지
bool isEEX = false; // 지수입력 있는지

// 키패드 라이브러리 설정 부분
Keypad kpdU = Keypad(makeKeymap(keysU), rowPinsU, colPinsU, ROWS, COLS);
Keypad kpdD = Keypad(makeKeymap(keysD), rowPinsD, colPinsD, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD. 작동이 되지 않으면 주소를 0x3F로 해볼 것


// 프로그램 시작점
void setup() {
    lcd.init(); //LCD 시작
    lcd.backlight();
    regX = (float64_t)0.0;
    regY = (float64_t)0.0;
    regZ = (float64_t)0.0;
    regT = (float64_t)0.0;
}

void loop() {
    char keyD = kpdD.getKey(); // 아래 키패드 입력받은 것을 keyD에 저장
    char keyU = kpdU.getKey(); // 윗 키패드 입력받은 것을 keyU에 저장
    if (keyD) { // 아래 키패드 처리
        switch (keyD) {
            case '1':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '1');
            }
            break;

            case '2':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '2');
            }
            break;

            case '3':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '3');
            }
            break;

            case '4':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '4');
            }
            break;

            case '5':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '5');
            }
            break;

            case '6':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '6');
            }
            break;

            case '7':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '7');
            }
            break;

            case '8':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '8');
            }
            break;

            case '9':
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '9');
            }
            break;

            case '0': //Shift: reset
            if (isShift) {
              clearMem(true);
            }
            else {
              if (!isBlockInput) szAppend(buffer, '0');
            }
            break;

            case '.': //Shift: 
            if (isShift) {

            }
            else {
              if (!isBlockInput) szAppend(buffer, '.');
            }
            break;

            case 'p': //Shift: 
            if (isShift) {

            }
            else {

            }
            break;

            case '-': //Shift: rad to deg
            bufferToRegX(true);
            if (isShift) {
              regX = calc_radToDegree(regX);
            }
            else {
              regX = fp64_add(regY, regX);
              rollDownReg(false);
            }
            break;

            case '+': //Shift: deg to rad
            bufferToRegX(true);
            if (isShift) {
              regX = calc_degreeToRad(regX);
            }
            else {
              regX = fp64_sub(regY, regX);
              rollDownReg(false);
            }
            break;

            case 'x': //Shift: x!
            bufferToRegX(true);
            if (isShift) {

            }
            else {
              regX = fp64_mul(regY, regX);
              rollDownReg(false);
            }
            break;

            case '/': //Shift: 
            bufferToRegX(true);
            if (isShift) {
              
            }
            else {
              //(regX == (float64_t)0.0)
              if (fp64_compare(regX, (float64_t)0.0) == (float64_t)0.0) {
                  errCode == ERR_DIVZERO
                  goto loop_err;
              }
              regX = fp64_div(regY, regX);
            }
            rollDownReg(false);
            break;
        }
        proc(); // 아래 키패드가 눌렸다면 처리 함수 호출
    } 
    else if (keyU) { // 윗 키패드 처리
        switch (keyU) {
            case BTN_LOG: //Shift: log2
            if (isShift) {
              
            }
            else {
              
            }
            break;

            case BTN_LN: //Shift: log_x(Y)

            break;

            case BTN_EX: //Shift: e

            break;

            case BTN_ROLLDOWN: //Shift: roll up reg
            if (isShift) {
              rollUpReg();
            }
            else {
              rollDownReg(true);
            }
            break;

            case BTN_SHIFT: //Shift: noShift
            if (!isShift) isShift = true;
            else isShift = false;
            break;

            case BTN_SIN: //Shift: asin

            break;

            case BTN_COS: //Shift: acos

            break;

            case BTN_TAN: //Shift: atan

            break;

            case BTN_PWR: //Shift: sq(x)

            break;

            case BTN_SQRT: //Shift: (x)th root of y

            break;

            case BTN_RECIPROCAL: //Shift: sto

            break;

            case BTN_EXCHANGEXY: //Shift: rcl

            break;

            case BTN_ENTER: //Shift: abs

            break;

            case BTN_CHS: //Shift: chys(change symbol of y)
            bufferToRegX(false);
            //(regX > (float64_t)0.0)
            if (fp64_compare(regX, (float64_t)0.0) == (float64_t)1.0) {
                shiftBuffer(RIGHT);
                buffer[0] = '-';
            }
            //(regX < (float64_t)0.0) {
            else if (fp64_compare(regX, (float64_t)0.0) == (float64_t)-1.0) {
                shiftBuffer(LEFT);
            }
            break;

            case BTN_EEX: //Shift: clear memory(for sto/rcl)

            break;

            case BTN_CLX: //Shift: clear
            if (isShift) {
              clearMem(false);
            }
            else {
              memset(buffer, 0, sizeof(buffer));
              regX = (float64_t)0.0;
            }
            break;
        }
        proc(); // 윗 키패드가 눌렸다면 처리 함수 호출
    }
    return;

    loop_err: // 루프 함수에서 생긴 오류 처리
    // 따로 연산이 필요하면 코드 넣기
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
    if (fp64_compare(regX, (float64_t)1.0e100) == (float64_t)1.0 || fp64_compare(regX, (float64_t)1.0e-100) == (float64_t)-1.0) {
        errCode = ERR_OOR;
    }
    // 다른 오류 처리 코드 넣기

    printLCD(MODE_ERR); // 오류 표시
}

void rollDownReg(bool isRBTN) { // 레지스터 하나씩 내리는 함수
      if (isRBTN) {
        float64_t tx = regX;
        regX = regY;
        regY = regZ;
        regZ = regT;
        regT = tx;
      }
      else {
        regY = regZ;
        regZ = regT;
        regT = 0;
      }
}

void rollUpReg() { // 레지스터 하나씩 올리는 함수
    float64_t tmp = tt;
    regT = regZ;
    regZ = regY;
    regY = regX;
    regX = tt;
}

void printLCD(byte mode) {
    char str[12] = { 0, };
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
        lcd.setCursor(12, 1);
        lcd.print("BUSY!");
        return;
    }
    if (mode == MODE_RES) { // 결과값을 표시하는 부분
        char *ptr = fp64_to_string(regX, -16, 12);
        lcd.print(ptr);
    }
    else if (mode == MODE_IN) { // 입력값을 표시하는 부분
        lcd.print(buffer);
        lcd.setCursor(13, 0);
        lcd.print(expBuf);
    }
    // 상태 정보를 아랫줄에 표시
    lcd.setCursor(0, 1); // 아랫줄 처음으로 커서 설정
    if (fp64_compare(regY, (float64_t)0.0) == (float64_t)0.0) { // Y 레지스터에 값이 있으면 Y 표시
        lcd.print('Y');
    }
    if (fp64_compare(regZ, (float64_t)0.0) == (float64_t)0.0) { // Z 레지스터에 값이 있으면 Z 표시
        lcd.setCursor(1, 1);
        lcd.print('Z');
    }
    if (fp64_compare(regT, (float64_t)0.0) == (float64_t)0.0) { // T 레지스터에 값이 있으면 T 표시
        lcd.setCursor(2, 1);
        lcd.print('T');
    }
    lcd.setCursor(0, 1); // 아랫줄 처음으로 커서 설정
    if (isShift) { // arc 눌렀으면 표시
        lcd.setCursor(4, 1);
        lcd.print("SHFT");
    }
    if (stomem != (float64_t)0.0) {
        lcd.setCursor(9, 1)
        lcd.print('M');
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
    regX = (float64_t)0.0; // X 레지스터의 값을 0으로 초기화
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
    buffer[11] = 0; // 버퍼 마지막은 반드시 null이 들어감
}

void bufferToRegX(bool clrBuffer) { // 버퍼의 값을 레지스터 X로 복사.
    char* eptr;
    regX = fp64_strtod(buffer, &eptr);
    if (clrBuffer) memset(buffer, 0, sizeof(buffer)); // 인수가 참일 때만 버퍼 지움
}

void clearMem(bool reset) { // 메모리 비우는 함수
    memset(buffer, 0, sizeof(buffer));
    regX = (float64_t)0.0;
    regY = (float64_t)0.0;
    regZ = (float64_t)0.0;
    regT = (float64_t)0.0;
    isBlockInput = false;
    isDecimal = false;
    op = 0;
    isShift = false;
    if (reset) {
      isBkLight = true;
    }
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

void szAppend(char *sz, const char ch) { // 글자를 문자열에 덧붙이는 함수
    char* ptr = sz;
    for (int i = 0; i < 11; i++) {
      if (*ptr == 0 && i < 10) { // 10번 미만에서 널문자 있을 때
        *ptr++ = ch;
        if (ch == '.') isDecimal = true; // 소수점 들어오면 소수점 마커를 참으로 바꿈
        if (!isDecimal) *ptr++ = '.'; // 소수점이 입력이 없으면 점을 마지막에 찍음
        *ptr = 0;
        return;
      }
      else if (*ptr == 0 && i == 10) { // 10번에서 널문자 있을 때는 점을 마지막에 찍지 않음
        *ptr++ = ch;
        *ptr = 0;
        
        return;
      }
      ptr++;
    }
}


/* 수학 함수 구현 */

//x의 절댓값을 내보내는 함수
float64_t calc_abs(float64_t x) {
  if (x > (float64_t)0.0) {
    return x;
  }
  else {
    return fp64_sub((float64_t)0.0, x);
  }
}

//x!을 출력하는 함수
//x는 0보다 크거나 같은 정수 범위
float64_t calc_facto(float64_t x) {
  if (x == (float64_t)0.0) {
    return (float64_t)1.0;
  }
  else {
    float64_t sum = (float64_t)1.0;
    /*
    for (float64_t i = (float64_t)2.0; i <= x; i++) { // while 문으로 바꿔야할 듯
      sum = fp64_mul(sum, i);
    }
    */
    float64_t i = (float64_t)2.0;
    while(1) {
      if (fp64_compare(i, x) == (float64_t)1.0) break;
      else {
        sum = fp64_mul(sum, i);
        i = fp64_add(i, (float64_t)1.0);
      }
    }
    return sum;
  }
}

/* x^y 값을 내보내는 함수
x는 0보다 크거나 같은 실수 범위 y는 정수 범위*/
float64_t calc_powInte(float64_t x, float64_t y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
  float64_t n = x;
  if (y == (float64_t)0.0) {
    n = (float64_t)1.0;
  }
  else if (y > (float64_t)0.0) {
    for (int i = 1; i < y; i++) {
      n = fp64_mul(n, x);
    }
  }
  else {
    for (int i = 1; i > y; i--) {
      n = fp64_div(n, x);
    }
  }
  return n;
}

float64_t calc_exp(float64_t x) {
  float64_t i = (float64_t)0.0;
  float64_t u = (float64_t)0.0;
  float64_t sum = (float64_t)0.0;
  float64_t memory = (float64_t)1.0;
  while (1) {
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
    memory = sum;
    u = fp64_div(calc_powInte(x, i), calc_facto(i));
    sum = fp64_add(sum, u);
    i = fp64_add(i, (float64_t)1.0);
  }
  return sum;
}

float64_t calc_ln(float64_t a) {
  float64_t n = (float64_t)1.0;
  while (1) {
    float64_t memory = n;
    // n = n - (calc_exp(n) - a) / calc_exp(n);//뉴튼 랩튼법
    n = fp64_sub(n, fp64_div(fp64_sub(calc_exp(n), a), calc_exp(n))); // 뉴턴 랩슨법
    if (fp64_compare(calc_abs(fp64_sub(memory, n)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
  }
  return n;
}

/*x^y를 출력하는 함수
x는 0보다 크거나 같은 실수 범위, y는 실수 범위*/
float64_t calc_pow(float64_t x, float64_t y) {
  float64_t YlnX = fp64_mul(y, calc_ln(x));
  return calc_exp(YlnX);
}

//sinx를 출력하는 함수
//x는 실수 범위
float64_t calc_sin(float64_t x) {
  float64_t sum = (float64_t)0.0;
  float64_t u = (float64_t)0.0;
  float64_t i = (float64_t)0.0;
  float64_t memory = (float64_t)1.0;
  while (1) {
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
    memory = sum;
    // u = calc_powInte(-1, i) * calc_powInte(x, 2 * i + 1) / calc_facto(2 * i + 1);
    u = fp64_div(fp64_mul(calc_powInte((float64_t)-1.0, i), calc_powInte(x, fp64_add(fp64_mul(i, (float64_t)2.0), (float64_t)1.0))), calc_facto(fp64_add(fp64_mul((float64_t)2.0, i), (float64_t)1.0)));
    sum = fp64_add(sum, u);
    i = fp64_add(i, (float64_t)1.0);
  }
  return sum;
}

//cosx를 출력하는 함수
//x는 실수 범위
float64_t calc_cos(float64_t x) {
  float64_t sum = (float64_t)0.0;
  float64_t u = (float64_t)0.0;
  float64_t i = (float64_t)0.0;
  float64_t memory = (float64_t)1.0;
  while (1) {
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
    memory = sum;
    //u = calc_powInte(-1, i) * calc_powInte(x, 2 * i) / calc_facto(2 * i);
    u = fp64_div(fp64_mul(calc_powInte((float64_t)-1.0, i), calc_powInte(x, fp64_mul((float64_t)2.0, i))), calc_facto(fp64_mul((float64_t)2.0, i)));
    sum = fp64_add(sum, u);
    i = fp64_add(i, (float64_t)1.0);
  }
  return sum;
}

//tanx를 출력하는 함수
//x는 실수 범위
float64_t calc_tan(float64_t x) {
  return fp64_div(calc_sin(x), calc_cos(x));
}

float64_t calc_arcsin(float64_t x) {
  float64_t i = (float64_t)0.0;
  float64_t u = (float64_t)0.0;
  float64_t sum = (float64_t)0.0;
  float64_t memory = (float64_t)1.0;
  float64_t temp1 = (float64_t)0.0
  while (1) {
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
    memory = sum;
    //u = calc_facto(2 * i) * calc_powInte(x, 2 * i + 1) / (calc_powInte(4, i) * calc_powInte(calc_facto(i), 2) * (2 * i + 1));
    u = fp64_mul(calc_facto(fp64_mul((float64_t)2.0, i)), calc_powInte(x, fp64_add(fp64_mul((float64_t)2.0, i), (float64_t)1.0)));
    u = fp64_div(u, calc_powInte((float64_t)4.0, i));
    u = fp64_mul(fp64_mul(calc_powInte(calc_facto(i), (float64_t)2.0), fp64_add(fp64_mul((float64_t)2.0, i), (float64_t)1.0)), u);
    sum = fp64_add(sum, u);
    i = fp64_add(i, (float64_t)1.0);
  }
  return sum;
}

float64_t calc_arccos(float64_t x) {
  //return pi / 2 - calc_arcsin(x);
  return fp64_sub(fp64_div(pi, (float64_t)2.0), calc_arcsin(x));
}

float64_t calc_arctan(float64_t x) {
  float64_t i = (float64_t)0.0;
  float64_t u = (float64_t)0.0;
  float64_t sum = (float64_t)0.0;
  float64_t memory = (float64_t)1.0;
  while (1) {
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == (float64_t)-1.0) {
      break;
    }
    memory = sum;
    //u = calc_powInte(-1, i) * calc_powInte(x, 2 * i + 1) / (2 * i + 1);
    u = fp64_div(fp64_mul(calc_powInte((float64_t)-1.0, i), calc_powInte(x, fp64_add(fp64_mul((float64_t)2.0, i), (float64_t)1.0))), fp64_add(fp64_mul((float64_t)2.0, i), (float64_t)1.0));
    sum = fp64_add(sum, u);
    i = fp64_add(i, (float64_t)1.0);
  }
  return sum;
}

float64_t calc_log(float64_t a) {
  //return calc_ln(a) / calc_ln(10);
  return fp64_div(calc_ln(a), calc_ln(10));
}

float64_t calc_sqrt(float64_t x) {
  //return calc_pow(x, 1.0 / 2);
  return calc_pow(x, fp64_div((float64_t)1.0, (float64_t)2.0));
}

float64_t calc_sqrtY(float64_t x, float64_t y) {
  //return calc_pow(x, 1.0 / y);
  return calc_pow(x, fp64_div((float64_t)1.0, y));
}

float64_t calc_log2(float64_t x) {
  //return calc_ln(x) / calc_ln(2);
  return fp64_div(calc_ln(x), calc_ln(2));
}

float64_t calc_logXY(float64_t x, float64_t y) {
  //return calc_ln(y) / calc_ln(x);
  return fp64_div(calc_ln(y), calc_ln(x));
}

float64_t calc_radToDegree(float64_t a) {
  //return a * 180 / pi;
  return fp64_div(fp64_mul(a, (float64_t)180.0), pi);
}

float64_t calc_degreeToRad(float64_t a){
  //return a * pi / 180;
  return fp64_div(fp64_mul(a, pi), (float64_t)180.0);
}
/*
float64_t calc_radToGon(float64_t a) {
  //return a / pi * 200;
  return fp64_mul(fp64_div(pi, (float64_t)200.0), a);
}

float64_t calc_gonToRad(float64_t a) {
  //return a * pi / 200;
  return fp64_div(fp64_mul(a, pi), (float64_t)200.0);
}
*/

//#pragma GCC pop_options
