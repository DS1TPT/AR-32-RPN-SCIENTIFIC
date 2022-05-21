/*
* AR-32 RPN SCIENTIFIC PROJECT 
* SOURCE CODE FOR UNO, BETA VERSION
*
* 아두이노 RPN(Reverse Polish Notation) 공학용 계산기 소스코드 - UNO등 8b AVR용
* 개발환경: 아두이노 우노(Arduino Uno)
* 개발 목표: 아두이노 우노로 세계최초의 휴대용 공학용 계산기인 HP-35의 현대화 버전을 만든다.
* HP-35의 버튼 배열에 기반하고, 기능 또한 HP-35에 있던 모든 기능을 포함한다. 단, 버튼이 32개로
* HP-35보다 적고 배열도 다르므로, 버튼 배열을 재배치하고 Shift로 여러 기능을 구현한다.
* HP-35의 35주년 기념판인 HP-35s의 기능을 다소 참고한다.
*
* ***기술적 정보***
*
* 빌드에 필요한 라이브러리: LiquidCrystal_I2C, Keypad, fp64lib
*
* 디스플레이: 16*2 LCD, 백라이트 있음
* 버튼 수: 32
* 레지스터 수: XYZT로 총 4개
* 추가 메모리: M(STO·RCL로 접근) 및 입력 버퍼(가수부, 지수부 분리)
* 사용 자료형: IEEE 754 DOUBLE-PRECISION FLOATING-POINT(fp64lib로 구현됨)
* 수식 입력방식: RPN 전용
* 전원: 9V 건전지 또는 USB
* 지원 연산기능 목록
* 사칙연산, 상용로그, 이진로그, log_x(y), 자연로그, 지수함수 e^x(exp)
* sin, cos, tan, arcsin, arccos, arctan,
* 거듭제곱, 제곱, 거듭제곱근, 제곱근, 역수,
* rad<->deg 변환, km<->mile 변환, l<->gal(US) 변환, kg<->lb(US) 변환, 섭씨<->화씨 변환, mm<->in 변환.
*
* 지원 기능 목록
* 레지스터 값 유무 표시, 메모리 값 유무 표시, 결과/입력 표시, 쉬프트 입력 및 표시,
* 레지스터 값 Roll(순차 이동), 오류 메시지 표시, 백라이트 ON/OFF.
* 
*/

/* LICENSE INFORMATION */
/*
Copyright 2021, Lee Geon-goo, Won Jong-wan.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// 값을 견주는 건 fp64_compare 리턴값을 fp64_to_int8 함수(반환형 char)로 받은 다음 해당 값이 -1,0,1 가운데
// 어느 것인지를 확인해서 해야 함. -1은 A<B, 0은 ==, 1은 A>B임.

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
#define NULL 0
#define BUF_LEN 13
#define EXP_LEN 3

// 상수 정의
const float64_t ACCURACY = (float64_t)0x3d719799812dea11LLU; //정확도 12자리
//const float64_t ACCURACY_LN = (float64_t)0x3c670ef54646d400LLU;
// 소수점 자리가 긴 상수값들은 라이브러리의 한계로 아래와 같이 16진수값 LLU로 직접 비트를 조작해야함..
const float64_t piNum = (float64_t)0x400921fb54442d18LLU; // 원주율 3.1415926535897932
const float64_t exponentialNum = (float64_t)0x4005bf0a8b145769LLU; // 자연로그의 밑 2.7182818284590452
const byte ROWS = 4; // 행 버튼 개수
const byte COLS = 4; // 열 버튼 개수

const char keysD[ROWS][COLS] = { // 윗 키패드
    {'7', '8', '9', '-'},
    {'4', '5', '6', '+'},
    {'1', '2', '3', 'x'},
    {'0', '.', 'p', '/'}
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
volatile float64_t stomem;
char buffer[BUF_LEN] = { 0, }; // 입력 버퍼(문자열)
char expBuf[EXP_LEN] = { 0, }; // 지수 입력 버퍼(문자열)
char outBuf[BUF_LEN] = { 0, }; // 출력 버퍼(문자열)
bool isOp = false; // 연산자 저장
byte errCode = NO_ERR; // 오류 코드 임시 저장
byte angleMode = ANGLE_RAD;
bool isShift = false; // Shift 상태 변수
bool isBkLight = true; // 백라이트 켜는지 끄는지 저장
bool isBlockInput = false; // 입력을 막는지 지정
bool isDecimal = false; // 소수점 있는지
bool isEEX = false; // 지수입력 있는지
bool isNegExp = false; // 지수가 음수인지

// 키패드 라이브러리 설정 부분
Keypad kpdU = Keypad(makeKeymap(keysU), rowPinsU, colPinsU, ROWS, COLS);
Keypad kpdD = Keypad(makeKeymap(keysD), rowPinsD, colPinsD, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD. 작동이 되지 않으면 주소를 0x3F로 해볼 것
const byte shiftArrow[8] = {B00000, B00100, B01110, B10101, B00100, B00100, B00100, B00000};

// 프로그램 시작점
void setup() {
    lcd.init(); //LCD 시작
    lcd.createChar(BTN_SHIFT, shiftArrow);
    lcd.backlight();
    regX = fp64_sd(0.0);
    regY = fp64_sd(0.0);
    regZ = fp64_sd(0.0);
    regT = fp64_sd(0.0);
    stomem = fp64_sd(0.0);
    lcd.setCursor(0, 0);
    lcd.print("AR-32");
    lcd.setCursor(2, 1);
    lcd.print("RPN SCIENTIFIC");
    delay(2000);
    printLCD(MODE_IN);
    Serial.begin(9600);
}

void loop() {
    char keyD = kpdD.getKey(); // 아래 키패드 입력받은 것을 keyD에 저장
    char keyU = kpdU.getKey(); // 윗 키패드 입력받은 것을 keyU에 저장
    if (keyD) { // 아래 키패드 처리
        switch (keyD) {
            case '1': //Shift: F -> C
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              if (fp64_compare(regX, fp64_sd(-459.67)) == 1) { //절대영도 보다 큰지 검사
							  regX = calc_cToF(regX);
							}
              else {
							  errCode = ERR_MATH;
                goto loop_err;
							}//END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '1', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '1', EXP_LEN);
              }
            }
            break;

            case '2': //Shift: C -> F
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: 섭씨 -> 화씨
              if (fp64_compare(regX, fp64_sd(-273.15)) == 1) { //절대영도 보다 높은지 검사
								regX = calc_cToF(regX);
							}
							else {
								errCode = ERR_MATH;
                goto loop_err;
							}//END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '2', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '2', EXP_LEN);
              }
            }
            break;

            case '3': //Shift: rad -> deg
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: rad -> deg
              regX = calc_radToDegree(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '3', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '3', EXP_LEN);
              }
            }
            break;

            case '4': //Shift: lb -> kg
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: lb -> kg
              regX = calc_ibToKg(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '4', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '4', EXP_LEN);
              }
            }
            break;

            case '5': //Shift: kg -> lb
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: kg -> lb
              regX = calc_kgToIb(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '5', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '5', EXP_LEN);
              }
            }
            break;

            case '6': //Shift: gal(US) -> L
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: gal(US) -> L
              regX = calc_galToL(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '6', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '6', EXP_LEN);
              }
            }
            break;

            case '7': //Shift: mile -> km
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: mile -> km
              regX = calc_mileToKm(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '7', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '7', EXP_LEN);
              }
            }
            break;

            case '8': //Shift: km -> mile
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: km -> mile
              regX = calc_kmToMile(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '8', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '8', EXP_LEN);
              }
            }
            break;

            case '9': //Shift: in -> mm
            if (isShift) {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: in -> mm
              regX = calc_inToMm(regX); //END
              isOp = true;
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '9', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '9', EXP_LEN);
              }
            }
            break;

            case '0': //Shift: reset
            if (isShift) {
              clearMem(true);
            }
            else {
              if (!isBlockInput && !isEEX) szAppend(buffer, '0', BUF_LEN);
              else if (isEEX && expBuf[1] == NULL) {
                szAppend(expBuf, '0', EXP_LEN);
              }
            }
            break;

            case '.': //Shift: backlight toggle
            if (isShift) {
              if (isBkLight) {
                isBkLight = false;
                lcd.noBacklight();
              }
              else {
                isBkLight = true;
                lcd.backlight();
              }
              isShift = false;
            }
            else {
              if (!isBlockInput && !isEEX && !isDecimal) szAppend(buffer, '.', BUF_LEN);
            }
            break;

            case 'p': //Shift: 2pi
            if (buffer[0] != 0) {
              bufferToRegX(true);
              rollUpReg(true);
            }
            if (isShift) {
              regX = fp64_mul(piNum, fp64_sd(2.0));
            }
            else {
              regX = piNum;
            }
            regToStr();
            isShift = false;
            break;

            case '-': //Shift: mm -> in
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: mm -> in
              regX = calc_mmToIn(regX); //EMD
            }
            else {
              regX = fp64_sub(regY, regX);
            }
            isOp = true;
            break;

            case '+': //Shift: L to gal(US)
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              regX = calc_LToGal(regX);
            }
            else {
              regX = fp64_add(regY, regX);
              rollDownReg(false);
            }
            isOp = true;
            break;

            case 'x': //Shift: deg -> rad
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: deg to rad
              regX = calc_degreeToRad(regX);
            }
            else {
              regX = fp64_mul(regY, regX);
              rollDownReg(false);
            }
            isOp = true;
            break;

            case '/': //Shift: x% of y
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              regX = fp64_mul(regY, fp64_div(regX, fp64_sd(100.0)));
            }
            else {
              if (fp64_compare(regX, fp64_sd(0.0)) == 0) {
                  errCode == ERR_DIVZERO;
                  goto loop_err;
              }
              else regX = fp64_div(regY, regX);
            }
            rollDownReg(false);
            isOp = true;
            break;
        }
        proc(); // 아래 키패드가 눌렸다면 처리 함수 호출
    } 
    else if (keyU) { // 윗 키패드 처리
        switch (keyU) {
            case BTN_LOG: //Shift: log_x(y)
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
            }
            if (isShift) {
              //참고 구현부1. TODO: log_x(y)
              regX = calc_logXY(regX, regY);
              rollDownReg(false); // 이 함수가 필요한 곳은 내가 따로 써놓았음.
            }
            else {
              //참고 구현부2. TODO: log(x)
              regX = calc_log(regX);
            }
            isOp = true;
            break;

            case BTN_LN: //Shift: log2
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
            }
            if (isShift) {
              //TODO: log2(x)
              regX = calc_logXY(fp64_sd(2.0), regX); //END
            }
            else {
              //TODO: ln(x)
              regX = calc_ln(regX); //END
            }
            rollDownReg(false);
            isOp = true;
            break;

            case BTN_EX: //Shift: e
            if (isShift) {
              if (buffer[0] != 0) {
                bufferToRegX(true);
                rollUpReg(true);
              }
              regX = exponentialNum;
              regToStr();
              isShift = false;
            }
            else {
              printLCD(MODE_BUSY);
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: e^x
              regX = calc_exp(regX); //END
              isOp = true;
            }
            break;

            case BTN_ROLLDOWN: //Shift: roll up reg
            if (isShift) {
              rollUpReg(false);
            }
            else {
              rollDownReg(true);
            }
            regToStr();
            isShift = false;
            break;

            case BTN_SHIFT: //Shift: noShift
            if (!isShift) isShift = true;
            else isShift = false;
            break;

            case BTN_SIN: //Shift: asin
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: ARCSIN
              if (fp64_compare(regX, fp64_sd(1.0)) == 1 || fp64_compare(regX, fp64_sd(-1.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else regX = calc_arcsin(regX);
            }
            else {
              //TODO: SIN
              regX = calc_sin(regX); //END
            }
            isOp = true;
            break;

            case BTN_COS: //Shift: acos
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: ARCCOS
              if (fp64_compare(regX, fp64_sd(1.0)) == 1 || fp64_compare(regX, fp64_sd(-1.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else regX = calc_arccos(regX);
            }
            else {
              //TODO: COS
              regX = calc_cos(regX); //END
            }
            isOp = true;
            break;

            case BTN_TAN: //Shift: atan
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: ARCTAN
              regX = calc_arctan(regX); //END
            }
            else {
              //TODO: TAN
              regX = calc_tan(regX); //END
            }
            isOp = true;
            break;

            case BTN_PWR: //Shift: x^2
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: x의 제곱
              if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else regX = calc_powInte(regX, fp64_sd(2.0)); //END
            }
            else {
              regX = calc_pow(regY, regX);
              rollDownReg(false);
            }
            isOp = true;
            break;

            case BTN_SQRT: //Shift: (x)th root of y
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: y의 x제곱근
              if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else {
                regX = calc_pow(regY, fp64_div(fp64_sd(1.0), regX)); //END
                rollDownReg(false);
              }
            }
            else {
              //TODO: 루트x
              if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else {
                regX = calc_root(regX); //END
              }
            }
            isOp = true;
            break;

            case BTN_RECIPROCAL: //Shift: abs
            printLCD(MODE_BUSY);
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              //TODO: abs(x)
              regX = calc_abs(regX); //END
            }
            else {
              //TODO: 1/x
              regX = fp64_div(fp64_sd(1.0), regX); //END
            }
            isOp = true;
            break;

            case BTN_EXCHANGEXY: //Shift: x!
            if (isShift) {
              if (buffer[0] != 0) bufferToRegX(true);
              //TODO: x!
              if (fp64_compare(regX, fp64_sd(0.0)) == -1) {
                errCode = ERR_MATH;
                goto loop_err;
              }
              else {
                regX = calc_facto(regX); //END
                isOp = true;
              }
            }
            else {
              float64_t tmpexchange = regX;
              regX = regY;
              regY = tmpexchange;
            }
            break;

            case BTN_ENTER: //Shift: sto
            if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              stomem = regX;
            }
            else {
              rollUpReg(true);
              return;
            }
            break;

            case BTN_CHS: //Shift: rcl
            //if (buffer[0] != 0) bufferToRegX(true);
            if (isShift) {
              if (buffer[0] != 0) bufferToRegX(true);
              rollUpReg(false);
              regX = stomem;
              regToStr();
              isShift = false;
            }
            else {
              if (isEEX) isNegExp = !isNegExp;
              else {
                if (buffer[0] != '-' && buffer[0] != NULL) {
                  shiftBuffer(RIGHT);
                  buffer[0] = '-';
                }
                else if (buffer[0] == '-') {
                  shiftBuffer(LEFT);
                }
              }
            }
            break;

            case BTN_EEX: //Shift: clear memory(for sto/rcl)
            if (isShift) {
              stomem = fp64_sd(0.0);
              isShift = false;
            }
            else {
              isEEX = true;
            }
            break;

            case BTN_CLX: //Shift: clear
            if (isShift) {
              clearMem(false);
            }
            else {
              clearX();
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
    if (!isOp) { // 연산자 입력 없음
        printLCD(MODE_IN); // 입력 출력
        return; // 함수 종료
    }
    else {
        isOp = false;
        regY = regX;
        printLCD(MODE_RES); // 결과 출력
    }
    isShift = false;
    /*
    if (getExp(&regX) > 99 || getExp(&regX) < -99 || getExp(&regY) > 99 || getExp(&regY) < -99 || getExp(&regZ) > 99 || getExp(&regZ) < -99 || getExp(&regT) > 99 || getExp(&regT) < -99 ) {
        errCode = ERR_OOR;
        goto proc_err;
    }
    */
    return;

    proc_err: // 처리 함수에서 생긴 오류 처리
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

void rollUpReg(bool isEnter) { // 레지스터 하나씩 올리는 함수
    float64_t tt = regT;
    regT = regZ;
    regZ = regY;
    regY = regX;
    if (!isEnter)regX = tt;
}

void printLCD(byte mode) {
    char str[12] = { 0, };
    lcd.clear();
    lcd.setCursor(0, 0); // 윗줄 처음으로 커서 설정
    if (mode == MODE_ERR) { // 오류 표시
        if (errCode == ERR_OOR) {
            lcd.print("OUT OF RANGE");
        }
        else if (errCode == ERR_DIVZERO) {
            lcd.print("DIVIDE BY 0");
        }
        else if (errCode == ERR_MATH) {
            lcd.print("MATH ERROR");
        }
        // 다른 오류 코드 표시 부분 구현하기

        lcd.setCursor(0, 1); //PRESS CLX 표시
        lcd.print("PRESS CLx");
        errWait(); // 오류 입력 대기
        return; // 함수 종료
    }
    if (mode == MODE_BUSY) { // 계산중 표시
        lcd.setCursor(10, 1);
        lcd.print(" BUSY!");
        return;
    }
    if (mode == MODE_RES) { // 결과값을 표시하는 부분
        char tmpOut[18] = { 0, };
        char outExp[4] = { 0, };
        memset(outBuf, 0, BUF_LEN);
        szCpy(tmpOut, sizeof(tmpOut), fp64_to_string(regX, 16, 10));
        char* p = szParse(tmpOut, "E");
        szCpyZero(outBuf, BUF_LEN, tmpOut);
        lcd.print(outBuf);
        if (p != NULL) {
          if (*p == '-') outExp[0] = '-';
          else outExp[0] = '+';
          szCpyZero(outExp + 1, EXP_LEN, ++p);
          lcd.setCursor(13, 0);
          lcd.print(outExp);
        }
    }
    else if (mode == MODE_IN) { // 입력값을 표시하는 부분
        if (buffer[0] == 0) lcd.print("0.");
        else lcd.print(buffer);
        if (expBuf[0] == 0 && isEEX) {
          lcd.setCursor(13, 0);
          lcd.print("_00");
        }
        else if (expBuf[0] != 0 && isEEX) {
          lcd.setCursor(13, 0);
          if (isNegExp) lcd.print('-');
          else lcd.print('+');
          lcd.setCursor(14, 0);
          lcd.print(expBuf); 
        }
    }
    // 상태 정보를 아랫줄에 표시
    lcd.setCursor(0, 1); // 아랫줄 처음으로 커서 설정
    if (fp64_compare(regY, fp64_sd(0.0)) != 0) { // Y 레지스터에 값이 있으면 Y 표시
        lcd.print('Y');
    }
    if (fp64_compare(regZ, fp64_sd(0.0)) != 0) { // Z 레지스터에 값이 있으면 Z 표시
        lcd.setCursor(1, 1);
        lcd.print('Z');
    }
    if (fp64_compare(regT, fp64_sd(0.0)) != 0) { // T 레지스터에 값이 있으면 T 표시
        lcd.setCursor(2, 1);
        lcd.print('T');
    }
    lcd.setCursor(0, 1); // 아랫줄 처음으로 커서 설정
    if (isShift && mode != MODE_RES) { // Shift 눌렀으면 표시
        lcd.setCursor(6, 1);
        lcd.write(BTN_SHIFT);
    }
    if (fp64_ds(stomem) != 0.0) {
        lcd.setCursor(8, 1);
        lcd.print('M');
    }
    if (mode == MODE_RES) { // 결과값 표시면 RESULT라고 표시
        lcd.setCursor(10, 1);
        lcd.print("RESULT");
    }
    else if (mode == MODE_IN) { // 입력중이면 INPUT 표시
        lcd.setCursor(10, 1);
        lcd.print(" INPUT");
    }
}

void errWait() {
    char keytmp = 0;
    while(1) {
        keytmp = kpdU.getKey();
        if (keytmp == BTN_CLX) break;
    }
    regX = fp64_sd(0.0); // X 레지스터의 값을 0으로 초기화
    memset(buffer, 0, BUF_LEN); // 입력을 모두 지움
    memset(expBuf, 0, EXP_LEN);
    isEEX = false;
    isShift = false;
    isNegExp = false;
    isOp = false; // 연산자 유무 마커 지움
}

/* 기능 함수 구현 */
void shiftBuffer(byte dir) { // 버퍼에서 문자를 한 방향으로 미는 함수, 순환 없음
    if (dir == RIGHT) {
        for (int i = BUF_LEN - 1; i >= 0; i--) {
            buffer[i + 1] = buffer[i];
        }
    }
    else if (dir == LEFT) {
        for (int i = 0; i < BUF_LEN - 1; i++) {
            buffer[i] = buffer[i + 1];
        }
    }
    buffer[BUF_LEN - 1] = 0; // 버퍼 마지막은 반드시 null이 들어감
}

void bufferToRegX(bool clrBuffer) { // 버퍼의 값을 레지스터 X로 복사.
    char* eptr;
    int totalLen = BUF_LEN + EXP_LEN + 1;
    char tmpBuf[totalLen] = { 0, };
    char* tmp = tmpBuf;
    szCpy(tmpBuf, BUF_LEN, buffer); // 가수 입력버퍼 복사
    while(1) { // 가수부 끝 구함
      if (*tmp == NULL) break;
      tmp++;
    }
    if (isEEX) { // 지수부가 있으면 지수 입력을 가수부 끝에 이어붙임
      *tmp++ = 'E';
      if (!isNegExp) *tmp++ = '+';
      else *tmp++ = '-';
      szCpy(tmp, EXP_LEN, expBuf);
    }

    regX = fp64_strtod(tmpBuf, &eptr);
    if (clrBuffer) { // 인수가 참일 때만 버퍼 지움
      memset(buffer, 0, BUF_LEN);
      memset(expBuf, 0, EXP_LEN);
      isEEX = false;
      isNegExp = false;
      isDecimal = false;
    }
}

void clearMem(bool reset) { // 메모리 비우는 함수
    memset(buffer, 0, BUF_LEN);
    memset(expBuf, 0, EXP_LEN);
    regX = fp64_sd(0.0);
    regY = fp64_sd(0.0);
    regZ = fp64_sd(0.0);
    regT = fp64_sd(0.0);
    isBlockInput = false;
    isEEX = false;
    isNegExp = false;
    isDecimal = false;
    isOp = false;
    isShift = false;
    if (reset) {
      isBkLight = true;
      stomem = fp64_sd(0.0);
    }
}

void clearX() {
    memset(buffer, 0, BUF_LEN);
    memset(expBuf, 0, EXP_LEN);
    regX = fp64_sd(0.0);
    isEEX = false;
    isNegExp = false;
    isDecimal = false;
}
void regToStr() { // regX에 새 값이 들어왔을 때, 그 값을 버퍼에 넣어줌
  if (fp64_compare(regX, fp64_sd(0.0)) == 0) {
    clearX();
    return;
  }
  char output[16];
  memset(buffer, 0, BUF_LEN); // 작업하기 전 버퍼를 비움
  memset(expBuf, 0, EXP_LEN);
  szCpy(output, sizeof(output), fp64_to_string(regX, 16, 10));
  char *p = output;
  while(*p != 0) {
    if (*p == '.') isDecimal = true;
    p++;
  }
  p = szParse(output, "E");
  szCpyZero(buffer, BUF_LEN, output);
  if (p == NULL) { // E 없음
    isEEX = false;
  }
  else {
    isEEX = true;
    if (*p == '-') isNegExp = true;
    else isNegExp = false;
    szCpyZero(expBuf, EXP_LEN, ++p);
  }
}

int getExp(float64_t* pReg) { // 지수부를 구해 정수로 반환하는 함수
  char tmp[18] = { 0, };
  szCpy(tmp, sizeof(tmp), fp64_to_string(*pReg, 16, 10));
  char* p = szParse(tmp, "E");
  if (p == NULL) return 0;
  else return atoi(p);
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

int szCmp(char* sz1, char* sz2) { // 문자열 비교 함수
    while (*sz1 == *sz2) {
        if (!(*sz1)) return *sz1 - *sz2;
        sz1++;
        sz2++;
    }
    return *sz1 - *sz2;
}

void szCpy(char* dst, unsigned size, char* src) {
    unsigned u = 0;
    for (u = 0; u < size - 1; u++) *dst++ = *src++;
    *dst = NULL;
}

void szCpyZero(char* dst, unsigned size, char* src) {
    unsigned u = 0;
    for (u = 0; u < size - 1; u++) {
      *dst++ = *src++;
      if (*src == NULL) break;
    }
    *dst = NULL;
}

void szAppend(char *sz, const char ch, int len) { // 글자를 문자열에 덧붙이는 함수
    char* ptr = sz;
    for (int i = 0; i < len; i++) {
      if ((*ptr == 0 || isDecimal == false && *ptr == '.' ) && *(ptr + 1) == 0 && i < (len - 1)) { // 10번 미만에서 널문자 있을 때
        *ptr++ = ch;
        if (ch == '.') isDecimal = true; // 소수점 들어오면 소수점 마커를 참으로 바꿈
        if (!isDecimal && !isEEX) *ptr++ = '.'; // 소수점이 입력이 없으면 점을 마지막에 찍음
        *ptr = 0;
        return;
      }
      else if ((*ptr == 0 || isDecimal == false && *ptr == '.' ) && *(ptr + 1) == 0 && i == (len - 1)) { // 10번에서 널문자 있을 때는 점을 마지막에 찍지 않음
        *ptr++ = ch;
        *ptr = 0;
        return;
      }
      ptr++;
    }
}


/* 수학 함수 구현 */

//#pragma GCC optimize ("-O0")
//#pragma GCC push_options

//x의 절댓값을 내보내는 함수
float64_t calc_abs(float64_t x) {
  if (fp64_compare(x, fp64_sd(0.0)) == 1) {
    return x;
  }
  else {
    return fp64_neg(x);
  }
}

//x!을 출력하는 함수
//x는 0보다 크거나 같은 정수 범위
float64_t calc_facto(float64_t x) {
  if (fp64_compare(x, fp64_sd(0.0)) == 0) {
    return fp64_sd(1.0);
  }
  else {
    float64_t sum = fp64_sd(1.0);
    /*
    for (float64_t i = (float64_t)2.0; i <= x; i++) { // while 문으로 바꿔야할 듯
      sum = fp64_mul(sum, i);
    }
    */
    float64_t i = fp64_sd(2.0);
    while(1) {
      if (fp64_compare(i, x) == 1) break;
      else {
        sum = fp64_mul(sum, i);
        i = fp64_add(i, fp64_sd(1.0));
      }
    }
    return sum;
  }
}

// x^y
// x >= 0  -inf<y(정수)<+inf
float64_t calc_powInte(float64_t x, float64_t y) { //pow를 만들기 위해 필요할 것으로 예상되어 미리 복제해둠.
  float64_t n = x;
  if (fp64_compare(y, fp64_sd(0.0)) == 0) {
    n = fp64_sd(1.0);
  }
  else if (fp64_compare(y, fp64_sd(0.0)) == 1) {
    for (long i = 1; fp64_compare(fp64_int32_to_float64(i), y) == -1; i++) {
      //n *= x;
      n = fp64_mul(n, x);
    }
  }
  else {
    for (long i = 1; fp64_compare(fp64_int32_to_float64(i), y) == 1; i--) {
      //n = n / x;
      n = fp64_div(n, x);
    }
  }
  return n;
}

//x - round(x)를 대체
//mod와 exp에 쓰임
float64_t calc_inteCut(float64_t x){
	float64_t x1 = fp64_round(x);
	float64_t result = fp64_sub(x, x1);
	if(fp64_compare(x1, x) <= 0){
		return result;
	}
	else{
		return fp64_add(result, fp64_sd(1.0));
	}
}

//실수 범위 모듈러 연산
//sin연산에 쓰임
float64_t calc_mod(float64_t x, float64_t y) {
	float64_t x0 = fp64_div(x, y);
	float64_t x1 = calc_inteCut(x0);
	return fp64_mul(x1, y);
}


float64_t calc_exp(float64_t x) {
  long n = 0;
  float64_t sum = fp64_sd(0.0);
  float64_t ex0 = calc_powInte(exponentialNum, fp64_trunc(x));
  float64_t xDiff = fp64_sub(x, fp64_trunc(x));
  float64_t sumPrev = fp64_sd(0.0);
  float64_t frac = fp64_sd(0.0);
  float64_t powd = fp64_sd(0.0);
  if (fp64_isinf(ex0) == 1) return ex0;
  while (1) {
    sumPrev = sum;
    frac = fp64_div(ex0, calc_facto(fp64_int32_to_float64(n)));
    powd = calc_powInte(xDiff, fp64_int32_to_float64(n));
    sum = fp64_add(sumPrev, fp64_mul(frac, powd));
    if (fp64_compare(calc_abs(fp64_sub(sumPrev, sum)), ACCURACY) == -1) break;
    n++;
  }
  return sum;
}

//x^y
//x>=0 , -inf<y<+inf
//exp, lnx 사용
float64_t calc_pow(float64_t x, float64_t y) {
  return calc_exp(fp64_mul(y, calc_ln(x)));
}

//x^(1/2)
//x>=0
float64_t calc_root(float64_t x) {
  float64_t n = fp64_div(x, fp64_sd(2.0));
  float64_t m0;
  int cnt = 0;
  float64_t outMemory = fp64_sd(0.0);
  float64_t memory = n;
  while (1) {
    memory = n;
    if (cnt % 2 == 1) {
      if (fp64_compare(outMemory, n) == 0) {
        break;
      }
      outMemory = n;
    }
    m0 = calc_powInte(n, fp64_sd(2.0));
    if (fp64_compare(m0, x) == 0) {
      break;
    }
    else {
      //n = (m0 + x) / (2 * n);
      n = fp64_div(fp64_add(m0, x), fp64_mul(fp64_sd(2.0), n));
      if (fp64_compare(calc_abs(fp64_sub(memory, n)), ACCURACY) == -1) {
        break;
      }
      cnt++;
    }
  }
  return n;
}

//ln함수의 부속품
//메클로린 연산 파트
float64_t calc_lnA(float64_t x) { //return ln(x+1)
  long cnt = 1;
  float64_t sum = fp64_sd(0.0);
  float64_t memory = sum;
  while (1) {
    memory = sum;
    //sum = sum + calc_powInte(-1.0, cnt + 1) * calc_powInte(x, cnt) / cnt;
    sum = fp64_add(sum, fp64_mul(calc_powInte(fp64_sd(-1.0), fp64_int32_to_float64(cnt + 1)), fp64_div(calc_powInte(x, fp64_int32_to_float64(cnt)), fp64_int32_to_float64(cnt))));
    //if (calc_abs(memory - sum) < acc) break;
    if (fp64_compare(calc_abs(fp64_sub(memory, sum)), ACCURACY) == -1) break;
    cnt++;
  }
  return sum;
}

//lnx
//x>0
float64_t calc_ln(float64_t x) {
  float64_t x0;
  //if (x >= 0.5 && x <= 1.5) { //lnA에 x-1 대입
  if (fp64_compare(x, fp64_sd(0.5)) >= 0 && fp64_compare(x, fp64_sd(1.5)) <= 0) {
    return calc_lnA(fp64_sub(x, fp64_sd(1.0)));
  }
  else if (fp64_compare(x, fp64_sd(2.0)) == 1) { //lnB에 대입 후 lnA 이용
    x0 = x;
  }
  else { //lnA에 1/1+x - 1 대입 후  - 붙임
    x0 = fp64_div(fp64_sd(1.0), x);
  }
  long cnt = 0;
  while (fp64_compare(x0, fp64_sd(2.0)) >= 0) {
    x0 = calc_root(x0);
    cnt++;
  }
  if (fp64_compare(x, fp64_sd(2.0)) == 1) {
    //return -calc_ln(1 / x0) * calc_powInte(2, cnt);
    return fp64_mul(fp64_neg(calc_ln(fp64_div(fp64_sd(1.0), x0))), calc_powInte(fp64_sd(2.0), fp64_int32_to_float64(cnt)));
  }
  else {
    //return calc_ln(1 / x0) * calc_powInte(2, cnt);
    return fp64_mul(calc_ln(fp64_div(fp64_sd(1.0), x0)), calc_powInte(fp64_sd(2.0), fp64_int32_to_float64(cnt)));
  }
}

float64_t calc_cosA(float64_t x) {
  float64_t n = fp64_sd(0.0);
  float64_t yn = fp64_sd(0.0);
  float64_t ynPrev = fp64_sd(0.0);
  float64_t numerator = fp64_sd(0.0);
  float64_t denominator = fp64_sd(0.0);
  // y(n) = ( (-1^n * x^(2n+1)) / (2n+1)! )
  while(1) {
    ynPrev = yn;
    numerator = fp64_mul(calc_powInte(fp64_sd(-1.0), n), calc_powInte(x, fp64_add(fp64_mul(fp64_sd(2.0), n), 1)));
    denominator = calc_facto(fp64_add(fp64_mul(fp64_sd(2.0), n), 1));
    yn = fp64_add(yn, fp64_div(numerator, denominator));
    if (fp64_compare(calc_abs(fp64_sub(ynPrev, yn)), ACCURACY) == -1) break;
    n = fp64_add(n, fp64_sd(1.0));
  }
  return yn;
}

float64_t calc_cos(float64_t x) { //x를 sinA의 유효범위 안으로 변환, 입력, 출력
   float64_t a = calc_mod(x, fp64_mul(fp64_sd(2.0), piNum));
   if (fp64_compare(x, fp64_sd(0.0)) < 0) {
      a = fp64_sub(a, fp64_mul(fp64_sd(2.0), piNum));
   }
   if (fp64_compare(a, fp64_sd(0.0)) >= 0 && fp64_compare(a, fp64_div(piNum, fp64_sd(2.0))) <= 0) {
      return calc_cosA(a);
   }
   else if (fp64_compare(a, fp64_div(piNum, fp64_sd(2.0))) == 1 && fp64_compare(a, piNum) <= 0) {
      return fp64_neg(calc_cosA(fp64_sub(piNum, a)));
   }
   else if (fp64_compare(a, piNum) == 1 && fp64_compare(a, fp64_div(fp64_mul(piNum, fp64_sd(3.0)), fp64_sd(2.0))) <= 0) {
      return fp64_neg(calc_cosA(fp64_sub(a, piNum)));
   }
   else {
      return calc_cosA(fp64_sub(fp64_mul(fp64_sd(2.0), piNum), a));
   }
}


//sin x
//-inf<x<+inf
//cos x에 의존
float64_t calc_sin(float64_t x) {
  return calc_cos(fp64_sub(x, fp64_div(piNum, fp64_sd(2.0))));
}

//tanx를 출력하는 함수
//x는 실수 범위
float64_t calc_tan(float64_t x) {
  return fp64_div(calc_sin(x), calc_cos(x));
}

//arcsinx
// -1<x<1

float64_t calc_arcsin(float64_t a) {
  float64_t xn = fp64_sd(0.0);
  long n = 0;
  float64_t xnPrev = xn;
  float64_t xnMem = fp64_sd(0.0);
  float64_t numerator = fp64_sd(0.0);
  float64_t denominator = fp64_sd(0.0);
  while(1) {
    xnPrev = xn;
    if (n % 2 == 1) {
      if (fp64_compare(xnMem, xnPrev) == 0) break;
      xnMem = xnPrev;
    }
    numerator = fp64_sub(calc_sin(xnPrev), a);
    denominator = calc_cos(xnPrev);
    xn = fp64_sub(xnPrev, fp64_div(numerator, denominator));
    if (fp64_compare(calc_abs(fp64_sub(xnPrev, xn)), ACCURACY) == -1) break;
    n++;
  } 
  return xn;
}

float64_t calc_arccos(float64_t x) {
  //return piNum / 2 - calc_arcsin(x);
  return fp64_sub(fp64_div(piNum, fp64_sd(2.0)), calc_arcsin(x));
}

float64_t calc_arctan(float64_t x) {
	float64_t index = fp64_sd(1.0);
  if (fp64_compare(x, fp64_sd(0.0)) == -1) index = fp64_sd(-1.0);
	//double t = (1 + (1 / calc_powInte(x, 2)));
  float64_t t = fp64_add(fp64_sd(1.0), fp64_div(fp64_sd(1.0), calc_powInte(x, fp64_sd(2.0))));
	//return index*calc_arcsin(calc_root(1 / t));
  return fp64_mul(index, calc_arcsin(calc_root(fp64_div(fp64_sd(1.0), t))));
}

//#pragma GCC pop_options

float64_t calc_log(float64_t a) {
  //return calc_ln(a) / calc_ln(10);
  return fp64_div(calc_ln(a), calc_ln(fp64_sd(10.0)));
}

float64_t calc_sqrt(float64_t x) {
  //return calc_pow(x, 1.0 / 2);
  return calc_pow(x, fp64_div(fp64_sd(1.0), fp64_sd(2.0)));
}

float64_t calc_sqrtY(float64_t x, float64_t y) {
  //return calc_pow(x, 1.0 / y);
  return calc_pow(x, fp64_div(fp64_sd(1.0), y));
}

/*
float64_t calc_log2(float64_t x) {
  //return calc_ln(x) / calc_ln(2);
  return fp64_div(calc_ln(x), calc_ln(2));
}
*/

float64_t calc_logXY(float64_t x, float64_t y) {
  //return calc_ln(y) / calc_ln(x);
  return fp64_div(calc_ln(y), calc_ln(x));
}

float64_t calc_radToDegree(float64_t a) {
  //return a * 180 / piNum;
  return fp64_div(fp64_mul(a, fp64_sd(180.0)), piNum);
}

float64_t calc_degreeToRad(float64_t a){
  //return a * piNum / 180;
  return fp64_div(fp64_mul(a, piNum), fp64_sd(180.0));
}

float64_t calc_fToC(float64_t f) {
	//return (f - 32) / 1.8;
   return fp64_div(fp64_sub(f, fp64_sd(32.0)), fp64_sd(1.8));
}

float64_t calc_cToF(float64_t c) {
	//return (c * 1.8) + 32;
   return fp64_add(fp64_mul(c, fp64_sd(1.8)), fp64_sd(32.0));
}

float64_t calc_kgToIb(float64_t kg) {
	//return kg / 0.453592;
   return fp64_div(kg, fp64_sd(0.453592));
}

float64_t calc_ibToKg(float64_t ib) {
	//return ib * 0.453592;
   return fp64_mul(ib, fp64_sd(0.453592));
}

float64_t calc_galToL(float64_t gal) {
	//return gal * 3.785411784;
   return fp64_mul(gal, fp64_sd(3.785411784));
}

float64_t calc_LToGal(float64_t L) {
	//return L / 3.785411784;
   return fp64_div(L, fp64_sd(3.785411784));
}

float64_t calc_mileToKm(float64_t mile) {
	//return mile * 1.60934;
   return fp64_mul(mile, fp64_sd(1.60934));
}

float64_t calc_kmToMile(float64_t km) {
	//return km / 1.60934;
   return fp64_div(km, fp64_sd(1.60934));
}

float64_t calc_inToMm(float64_t in) {
	//return in * 25.4;
   return fp64_mul(in, fp64_sd(25.4));
}

float64_t calc_mmToIn(float64_t mm) {
	//return mm / 25.4;
   return fp64_div(mm, fp64_sd(25.4));
}

//#pragma GCC pop_options
