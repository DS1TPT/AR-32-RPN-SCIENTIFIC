// 아직 틀을 다 짜지 않았음!!

/* 구현할 때 요구사항 및 주의사항
*  1. 0으로 나누기와 같은 건 명령어가 들어왔을 때 오류를 띄우고 막아야 함.
*  -> 오류는 위 왼쪽에서 ERROR만 띄우는 걸로 충분함.
*  2. RPN으로 구현함.
*  3. 제곱(SQ)은 X 레지스터의 값의 Y 제곱으로 구현함
*  4. 입력은 12자리까지만 받을 것. 음수이고 지수도 음수이면 -123456789012-01과 같이 표시돼야 함.
*  4-1. 입력을 받을 때 다음 자리에 .을 표시할 것(12가 눌렸다면 12. 표시, 3을 누르면 123. 표시)
*  4-2. 소수점이 눌렸다면 다음 자리에 .을 표시하지 않음.
*  5. 입력 레지스터가 곧 X 레지스터임에 유의. 
*  5-1 결과값은 결과 출력 후 키 입력 전까지는 X 레지스터에 들어가고, 키 입력 후 Y로 올라감.
*  -> 500 엔터 20 누르면 Y에 500, X는 20임.
*  6. math.h 써도 됨. 단, 이걸 쓴다면 "상수 정의"에서 pi 상수는 주석처리할 것.
*/

#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>

#define BTN_SQ '^'
#define BTN_LOG 'L'
#define BTN_LN 'N'
#define BTN_EX 'e'
#define BTN_CLR 'C'
#define BTN_SQRT 'R'
#define BTN_ARC 'A'
#define BTN_SIN 's'
#define BTN_COS 'c'
#define BTN_TAN 't'
#define BTN_RECIPROCAL '|'
#define BTN_EXCHANGEXY 'X'
#define BTN_CHS 'H'
#define BTN_EEX 'E'
#define BTN_CLX 'O'
#define BTN_ENTER '='

#define MODE_RES 0
#define MODE_IN 1
#define MODE_BUSY 2
#define MODE_ERR 3

#define NO_ERR 0 // 오류 없음
#define ERR_OOR 1 // OUT OF RANGE 오류
#define ERR_DIVZERO 2 // 0으로 나누기 오류
// 다른 오류도 정의할 것


// 상수 정의
const byte ROWS = 4; // 행 버튼 개수
const byte COLS = 4; // 열 버튼 개수
const char keysU[ROWS][COLS] = { // 윗 키패드
    {'1','2','3','-'},
    {'4','5','6','+'},
    {'7','8','9','x'},
    {'0','.','p','/'}
};
const char keysD[ROWS][COLS] = { // 아래 키패드
    {BTN_LOG, BTN_LN, BTN_EX, BTN_CLR},
    {BTN_ARC, BTN_SIN, BTN_COS, BTN_TAN},
    {BTN_SQ, BTN_SQRT, BTN_RECIPROCAL, BTN_EXCHANGEXY},
    {BTN_ENTER, BTN_CHS, BTN_EEX, BTN_CLX};
};
const double pi = 3.141592653589793238; // math.h 쓰면 주석처리할 것
const byte rowPinsD[ROWS] = { 0, 1, 2, 3 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsD[COLS] = { 4, 5, 6, 7 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 
const byte rowPinsU[ROWS] = { 8, 9, 10, 11 }; // R1 ~ R4 차례대로 연결한 디지털 핀번호
const byte colPinsU[COLS] = { 12, 13, 14, 15 }; // C1 ~ C4 차례대로 연결한 디지털 핀번호 
// 14, 15번 핀은 각각 A0, A1 핀임.


// 전역변수 목록
volatile double regX, regY, regZ, regT; // 레지스터 XYZT, 수시로 값이 바뀔 수 있어 최적화 제외
bool isArc = false; // ARC 버튼이 눌렸는지를 저장하는 상태 변수
char numInput[13] = { 0, }; // 입력을 저장하는 문자열
char operator = 0; // 연산자를 저장하는 변수
byte errCode = NO_ERR;

LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD. 작동이 되지 않으면 주소를 0x27로 해볼 것
// 키패드 라이브러리 설정 부분
Keypad kpdU = Keypad(makeKeymap(keysU), rowPinsU, colPinsU, ROWS, COLS);
Keypad kpdD = Keypad(makeKeymap(keysD), rowPinsD, colPinsD, ROWS, COLS);

void setup() {
    lcd.begin(); //LCD 시작
    regX = 0.0;
    regY = 0.0;
    regZ = 0.0;
    regT = 0.0;
}

void loop() {
    char keyD = kpdD.getKey(); // 아래 키패드 입력받은 것을 keyD에 저장
    char keyU = kpdU.getKey(); // 윗 키패드 입력받은 것을 keyU에 저장
    if (keyD) {
        // 아래 키패드 처리
        proc(); // 아래 키패드가 눌렸다면 처리 함수 호출
    } 
    else if (keyU) {
        // 윗 키패드 처리
        if (keyU == BTN_ARC) {
            if (!isArc) isArc = true;
            else isArc = false;
        }
        proc(); // 윗 키패드가 눌렸다면 처리 함수 호출
    }
    else return;
}

void proc() { // 처리 함수
    if (operator) { // 연산자 입력 받음
        printLCD(MODE_BUSY); // 계산중이라고 띄워놓고 연산 시작
        // 연산을 처리하는 부분을 아래에 적기
    }
    else { // 연산자 입력 없음
        printLCD(MODE_IN); // 입력 출력
        return; // 함수 종료
    }
    printLCD(MODE_RES); // 결과 출력

    proc_err: // 오류 처리
    if (regX == 0.0 && operator == '/') { // 0으로 나누기 오류
        errCode = ERR_DIVZERO;
    }
    else if (regX >= 1.0e+100 || regX <= 1.0e-100 ) { // 범위 초과 오류
        errCode = ERR_OOR;
    }
    // 다른 오류 처리 코드 넣기

    printLCD(MODE_ERR); // 오류 표시
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
        // 다른 오류 코드 표시 부분 구현하기

        lcd.setCursor(0, 1); //PRESS CLX 표시
        lcd.print("PRESS CLX");
        char keytmp = 0;
        while(1) {
            keytmp = kpdU.getKey();
            if (keytmp == BTN_CLX) break;
        }
        delay(500); // 0.5초 기다림
        regX = 0.0; // X 레지스터의 값을 0으로 초기화
        for (i = 0; i < 13; i++) { // 입력을 모두 지움
            numInput[i] = 0;
        }
        operator = 0; // 연산자 지움
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
        lcd.print(numInput);
    }
    // 상태 정보를 아랫줄에 표시
    lcd.setCursor(0, 1) // 아랫줄 처음으로 커서 설정
    if (regY != 0.0) {
        lcd.print('Y');
    }
    if (regZ != 0.0) {
        lcd.setCursor(1, 1);
        lcd.print('Z');
    }
    if (regT != 0.0) {
        lcd.setCursor(2, 1);
        lcd.print('T');
    }
    if (isArc) {
        lcd.setCursor(4, 1);
        lcd.print("arc");
    }
    if (mode == MODE_RES) {
        lcd.setCursor(11, 1);
        lcd.print("RESULT");
    }
    else if (mode == MODE_IN) {
        lcd.setCursor(12, 1);
        lcd.print("INPUT");
    }
}
