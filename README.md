# AR-32 RPN SCIENTIFIC PROJECT

개발중: 파일 이름이 won 으로 시작하면 원종완이 쓴 코드임




아두이노 RPN(Reverse Polish Notation) 공학용 계산기 소스코드 - UNO등 8b AVR용

개발환경: 아두이노 우노(Arduino Uno)

개발 목표: 아두이노 우노로 세계최초의 휴대용 공학용 계산기인 HP-35의 현대화 버전을 만든다.
HP-35의 버튼 배열에 기반하고, 기능 또한 HP-35에 있던 모든 기능을 포함한다. 단, 버튼이 32개로
HP-35보다 적고 배열도 다르므로, 버튼 배열을 재배치하고 Shift로 여러 기능을 구현한다.
HP-35의 35주년 기념판인 HP-35s의 기능을 다소 참고한다.


 ***기술적 정보***

빌드에 필요한 라이브러리: LiquidCrystal_I2C, Keypad, fp64lib

디스플레이: 16*2 LCD, 백라이트 있음

버튼 수: 32

레지스터 수: XYZT로 총 4개

추가 메모리: M(STO·RCL로 접근) 및 입력 버퍼(가수부, 지수부 분리)

사용 자료형: IEEE 754 DOUBLE-PRECISION FLOATING-POINT(fp64lib로 구현됨)

수식 입력방식: RPN 전용

전원: 9V 건전지 또는 USB

지원 연산기능 목록

사칙연산, 상용로그, 이진로그, log_x(y), 자연로그, 지수함수 e^x(exp)

sin, cos, tan, arcsin, arccos, arctan, 거듭제곱, 제곱, 거듭제곱근, 제곱근, 역수,
rad<->deg 변환, km<->mile 변환, l<->gal(US) 변환, kg<->lb(US) 변환, 섭씨<->화씨 변환, mm<->in 변환.

***지원 기능 목록***

레지스터 값 유무 표시, 메모리 값 유무 표시, 결과/입력 표시, 쉬프트 입력 및 표시,
레지스터 값 Roll(순차 이동), 오류 메시지 표시, 백라이트 ON/OFF.
