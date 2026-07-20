# AXI4-Lite SoC Design — MicroBlaze 기반 Custom Peripheral 통합 프로젝트

Basys3(Artix-7) FPGA 위에 **MicroBlaze + AXI4-Lite 버스 중심의 SoC**를 구성하고, 직접 설계한 5종 Custom IP(GPIO / Timer / UART / I2C Master / SPI Master)로 다양한 주변장치를 하나의 모드형 임베디드 애플리케이션으로 통합 제어한 프로젝트입니다. 핵심 블록인 Custom GPIO IP에 대해서는 **SystemVerilog UVM 검증 환경**을 구축하여 Regression **Fail 0건, Functional Coverage 100%**를 달성했습니다.

> 진행 기간: 2026.06.22 ~ 2026.06.30 · 개발: 정광근 ([@fourevere](https://github.com/fourevere))
> 상세 내용은 [완료 보고서(PDF)](docs/Final_Report.pdf) 참고

---

## 1. 프로젝트 개요

기존에 개별 RTL 모듈 + FSM으로 구현했던 기능들을 **프로세서 기반 SoC 구조**로 재설계하여, 하드웨어 IP와 소프트웨어 애플리케이션의 역할을 분리하고 memory-mapped register를 통해 소프트웨어가 하드웨어를 제어하는 임베디드 시스템의 표준 개발 흐름을 구현했습니다.

| 구분 | 사양 |
|---|---|
| System Clock | 100 MHz |
| CPU | MicroBlaze soft-processor (local memory 128KB, standalone) |
| Bus | AXI4-Lite (AXI Interconnect 기반 memory-mapped I/O) |
| Custom IP | GPIO ×5, Timer, UART, I2C Master, SPI Master |
| System Tick | Custom Timer, PSC=99 / ARR=999 → 1ms interrupt |
| 주변장치 | SR04(초음파), LCD1602(I2C), RFID-RC522(SPI), FND, LED, Button |
| USB Serial | Xilinx axi_uartlite, 115200 bps |
| Verification | UVM 기반 GPIO IP 검증 환경 (agent / scoreboard / functional coverage) |
| 개발 환경 | Vivado 2020.2, Vitis 2020.2, Verilog / SystemVerilog(UVM) / C (BareMetal) |

### 동작 모드 (SW[2:0])

| SW[2:0] | 모드 | 동작 |
|---|---|---|
| `000` | Stopwatch | FND에 스톱워치 표시, BTNU run/stop · BTND clear, BTNL/BTNR로 Custom UART 명령 전송 |
| `001` | Current Time | LCD에 현재 시간 `HH:MM:SS.t` (0.1초 해상도) 표시 |
| `010` | RFID Attendance | RC522 카드 태그 시 등록 이름과 IN/OUT 시간 표시, 미등록 카드는 UID 표시, 1.5초 중복 쿨다운 |
| `011` | SR04 Distance | BTNU 입력 시 초음파 거리 1회 측정(mm), 5회 측정 중간값 필터 |
| `100` | Time Setting | USB terminal에서 `T=HH:MM:SS` 입력으로 시간 설정 |

---

## 2. 시스템 아키텍처

MicroBlaze가 AXI4-Lite Interconnect를 통해 모든 주변장치 IP를 memory-mapped 방식으로 제어합니다. 각 IP의 인터럽트는 xlconcat → AXI INTC를 거쳐 MicroBlaze로 단일 인터럽트로 전달됩니다.

```
                        ┌──────────────┐
                        │  MicroBlaze  │◄─── AXI INTC ◄── xlconcat ◄── Timer/UART/I2C/SPI intr
                        └──────┬───────┘
                               │ AXI4-Lite Interconnect
   ┌────────┬────────┬────────┼────────┬────────┬────────┬──────────┐
   ▼        ▼        ▼        ▼        ▼        ▼        ▼          ▼
 GPIOA~E  Timer   Custom    I2C      SPI     axi_     (FND, LED,  ...
 (×5)     (1ms)   UART     Master   Master  uartlite   Button)
   │                │        │        │        │
  FND/LED/SW/    PMOD      LCD1602  RFID-    USB
  SR04 TRIG/ECHO UART      (I2C)    RC522   Terminal
```

### AXI4-Lite 주소 맵

| Block | Base Address | 역할 |
|---|---|---|
| `axi_uartlite_0` | `0x4060_0000` | USB serial terminal (115200 bps) |
| `gpio_0` (GPIOA) | `0x44A0_0000` | FND segment 출력 |
| `gpio_1` (GPIOB) | `0x44A1_0000` | FND digit select + board button |
| `gpio_2` (GPIOC) | `0x44A2_0000` | LED low byte |
| `gpio_3` (GPIOD) | `0x44A3_0000` | LED high byte |
| `timer_0` | `0x44A4_0000` | 1ms system tick |
| `uart_0` | `0x44A5_0000` | Custom UART (PMOD) |
| `i2c_master_0` | `0x44A6_0000` | LCD1602 I2C |
| `spi_0` | `0x44A7_0000` | RFID-RC522 SPI |
| `gpio_4` (GPIOE) | `0x44A8_0000` | 모드 스위치 + SR04 TRIG/ECHO |

### 소프트웨어 계층 구조 (Vitis BareMetal)

C 구조체와 RTL 레지스터 맵을 1:1로 매핑하는 HAL을 바탕으로 4계층으로 분리했습니다.

| 계층 | 폴더 | 책임 |
|---|---|---|
| **HAL** | `Timer_Uart/src/HAL` | AXI register 직접 접근 및 low-level IP 제어 (GPIO/TMR/UART/UARTLITE/I2C/SPI) |
| **driver** | `Timer_Uart/src/driver` | 디바이스 수준 기능 (Button, FND, LED, LCD1602, RC522, SR04, ModeSwitch) |
| **common** | `Timer_Uart/src/common` | 공용 delay 및 interrupt 지원 |
| **ap** | `Timer_Uart/src/ap` | 애플리케이션 모드 로직 (StopWatch, Clock, Attendance, Distance, TimeSetting, ModeManager) |

여러 모드가 하나의 LCD를 공유하므로 **ModeManager가 LCD Ownership을 관리**합니다. 모드 전환 시 이전 모드의 화면 접근 권한을 해제(Disable)한 뒤 새 모드가 화면을 clear하고 다시 그리는 구조로, 백그라운드 갱신이 다른 모드 화면을 덮어쓰는 문제를 구조적으로 방지했습니다.

---

## 3. Custom IP 설계

5종 IP 모두 AXI4-Lite slave 인터페이스로 직접 설계했으며, 총 34개 레지스터를 데이터시트 형식(offset / reset value / bit field / 접근권한)의 Reference Manual로 문서화했습니다. 상세 레지스터 맵은 [완료 보고서](docs/Final_Report.pdf) 4장 참고.

| IP | 주요 레지스터 | 특징 |
|---|---|---|
| **GPIO** (`ip_repo/gpio_1.0`) | CR / IDR / ODR | bit 단위 방향 제어 tri-state 구조, 5개 인스턴스로 재사용 |
| **Timer** (`ip_repo/Timer_1.0`) | CR / PSC / ARR / CNT | 1ms 시스템 tick 생성, CNT 직접 read로 us 단위 시간 측정(SR04) 지원 |
| **UART** (`ip_repo/uart_1.0`) | SR / TDR / RDR / CR | PMOD 경유 외부 통신, RX interrupt 지원 |
| **I2C Master** (`ip_repo/i2c_master_1.0`) | CR / SR / TXDATA 등 8종 | LCD1602 구동, 하드웨어 워치독 + 소프트웨어 timeout 이중 안전장치 |
| **SPI Master** (`ip_repo/SPI_1.0`) | SR / TDR / RDR / CR | **SS_HOLD** 기능으로 RC522의 multi-byte frame 동안 SS low 유지 |

---

## 4. UVM 검증 (Custom GPIO IP)

SoC에서 5회 인스턴스되어 재사용되는 Custom GPIO IP를 DUT로 하여 sequence–driver–monitor–scoreboard–coverage 구조의 UVM 테스트벤치를 구축했습니다. (`gpio_uvm/`)

- **Driver**: AXI4-Lite 5채널 handshake 구현. CR 방향에 따라 `ext_drive_en = ~CR`로 입력 bit를 항상 외부에서 구동하여 high-Z 부정값에 의한 오탐지를 원천 배제
- **Scoreboard**: `(CR & ODR) | (~CR & ext_drive_data)` 기대 핀 값 산출 후 CR/ODR/IDR/io_port 일괄 자동 비교
- **Coverage**: scenario / direction / ODR / IDR / wstrb / completed coverpoint로 검증 완성도를 정량 확인

**10종 시나리오** (reset 기본값, 전체 입력/출력, mixed 방향, byte-strobe 부분 write, 방향 토글, 읽기 전용 레지스터 write 무시, back-to-back, 제약 랜덤 등)를 directed + constrained-random(150회)으로 구성한 full regression 실행 결과:

> ✅ **Scoreboard Fail 0건 · Functional Coverage 100% (illegal bin hit 0건)**

---

## 5. 저장소 구조

```
├── ip_repo/               # Vivado IP 패키지 (AXI4-Lite Custom IP 5종)
│   ├── gpio_1.0/          #   GPIO IP (RTL + driver + example design)
│   ├── Timer_1.0/         #   Timer IP
│   ├── uart_1.0/          #   UART IP
│   ├── i2c_master_1.0/    #   I2C Master IP
│   └── SPI_1.0/           #   SPI Master IP
├── gpio_uvm/              # GPIO IP UVM 검증 환경
│   ├── rtl/               #   DUT (gpio_v1_0)
│   └── tb/                #   UVM testbench (if/pkg/seq/driver/monitor/scoreboard/coverage/test)
├── Timer_Uart/            # Vitis BareMetal application
│   └── src/               #   HAL / driver / common / ap 4계층
└── docs/
    ├── Final_Report.pdf   # 프로젝트 완료 보고서 (34p, 레지스터맵 포함)
    └── daily-logs/        # 일일 개발 일지 (2026-06-22 ~ 06-29)
```

---

## 6. 실행 방법

1. Vivado 2020.2에서 `ip_repo`를 IP repository로 추가하고 block design 구성 → synthesis / implementation / bitstream 생성 → XSA export
2. Vitis 2020.2에서 platform 생성 후 `Timer_Uart/src`로 application 빌드
3. Basys3 연결 → FPGA program → `Launch on Hardware`
4. 115200 bps serial terminal 오픈 → SW[2:0]으로 모드 선택

UVM 시뮬레이션은 `gpio_uvm/tb/gpio_tb_top.sv`를 top으로 컴파일 후 `+UVM_TESTNAME=gpio_full_regression_test`로 실행합니다.

---

## 7. 트러블슈팅 하이라이트

10건의 트러블슈팅을 원인 분석과 함께 [완료 보고서](docs/Final_Report.pdf) 7장에 기록했습니다. 대표 사례:

- **SR04 측정값 불안정** → SW 루프 딜레이 기반 측정을 폐기하고 Timer CNT 레지스터 직접 read 기반 us 측정(`SR04_Micros()`) + 5회 중간값 필터로 전면 재설계
- **비트스트림/XSA 플랫폼 불일치** → 하드웨어 스펙 변경 시 XSA export 및 Vitis 플랫폼 갱신을 원칙화하여 재발 방지
- **부팅 순서 데드락** → interrupt controller / timer 기동을 main() 최우선으로 재배치하는 startup flow 재설계
- **RC522 REQA ProtocolErr** → short frame(TxLastBits=7) 설정을 driver 내부로 강제 편입하여 실수 가능성 제거

---

## 8. 한계 및 향후 개선

- 폴링 기반 드라이버 → 하드웨어 인터럽트 연동 비동기 드라이버로 개선 예정
- RC522 1-byte 단위 접근 → FIFO burst 다중 byte 연속 읽기 적용 예정
- UVM 검증 범위를 I2C/SPI 등 나머지 IP로 확장하고 regression/coverage 수집 자동화 계획
- 카드 사용자 정보의 정적 배열 관리 → terminal 기반 런타임 등록/삭제 구조로 확장 계획

---

## 정오표 (Errata)

프로젝트 종료 후 산출물을 재검토하면서 발견한 오류를 기록합니다. `docs/`의 보고서·일지 원본은 제출 당시 상태를 보존하며, 코드는 저장소에서 수정했습니다.

**소스코드 (저장소에서 수정 완료)**

- 함수명 오타 `SetupInterrupSystem()` → `SetupInterruptSystem()` (main.c / interrupt.c / interrupt.h). 완료 보고서 3.3.4절과 7.9절에는 수정 전 이름으로 인용되어 있습니다.
- `Attendance.c`의 등록 사용자 중 타인의 실명을 "GUEST USER"로 익명화했습니다. 보고서·발표의 보드 시연 화면에 표시되는 이름과는 다를 수 있습니다.

**완료 보고서 (docs/Final_Report.pdf)**

- 3.1.1절 AXI4-Lite 주소 맵 표에 AXI Interrupt Controller(`axi_intc`, base `0x4120_0000`)가 누락되어 있습니다. 인터럽트 경로 설명(3.1절)에는 존재합니다.

**발표자료 (제출본 PPTX 기준 — 용량 문제로 저장소에는 미포함)**

- 7장 동작 모드 표에 5번째 모드인 Time Setting(SW=100)이 누락되어 있습니다. 보고서 3.2.1절과 코드(ModeManager)는 5개 모드가 맞습니다.
- 17장 UVM 설명은 발표용으로 단순화된 것입니다. 실제 sequence는 3종 directed 테스트가 아니라 10종 시나리오(reset_default ~ random) + full regression 구조이며, monitor는 AXI 버스를 직접 샘플링하는 것이 아니라 driver가 sideband 신호로 발행한 결과를 수집합니다(보고서 5.2~5.3절의 서술이 정확합니다).
- 20장 트러블슈팅의 "RC522 VER=0x00" 원인 설명(SS가 address/data 사이에 해제 → SS_HOLD 추가)과 보고서 7.10절의 원인 설명(SDA=SS 핀 배선 오류 → JA4 교정)은 **서로 다른 두 사건**입니다. SS_HOLD는 설계 단계에서 반영한 기능(보고서 4.5절)이고, 보드 브링업 단계의 VER=0x00은 배선 문제였습니다.
- 10장 제목 "adress map" → "address map", 12장 UART CR 설명의 "rx_id" → "rx_ie", 21장 결론의 IP 나열에 Timer 누락(GPIO/Timer/UART/I2C/SPI 5종이 맞음).
- 10장 우측 주소맵 요약 표에는 `axi_uartlite`(0x4060_0000)가 빠져 있습니다(좌측 Vivado 캡처에는 존재).
