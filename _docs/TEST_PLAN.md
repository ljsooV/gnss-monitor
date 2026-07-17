# GNSS Monitor 시험 계획

## 자동 시험

```powershell
ctest --preset x64-Debug
```

검증 항목:

- 위도·경도 십진수 변환
- 정상/불일치 체크섬 오류 체크
- 정상 GGA/RMC 파싱
- GGA/RMC No Fix 처리
- 누락 필드, 잘못된 방향, 숫자 변환 오류
- CRLF 및 LF 문장 조립
- 분할 수신, 다중 문장, 완성+미완성 조합

## 파일 통합 시험

```powershell
binary/build/x64-Debug/gnss-monitor.exe file _samples/sample.nmea
binary/build/x64-Debug/gnss-monitor.exe file _samples/no_fix.nmea
```

확인 사항:

1. `gga.csv`, `rmc.csv`가 생성된다.
2. 헤더가 한 번만 기록된다.
3. 정상 Fix 좌표가 CSV에 기록된다.
4. No Fix 행의 좌표 관련 필드가 비어 있다.
5. 실행 종료 후 처리 통계가 출력된다.

## 실외 장비 시험

1. USB GNSS를 노트북 COM 포트에 연결한다.
2. 안테나 면을 하늘 방향으로 두고 실외에서 실행한다.
3. GGA Fix Quality가 1 이상인지 확인한다.
4. 수신 위성 수, HDOP, 위도·경도, 고도가 출력되는지 확인한다.
5. 5분 이상 실행하여 CSV 행이 지속적으로 증가하는지 확인한다.
6. Ctrl+C로 종료하고 정상 종료 메시지와 손상되지 않은 CSV를 확인한다.

## 증적

- 콘솔 정상 Fix 화면
- 콘솔 No Fix 화면
- `gga.csv`, `rmc.csv`
- CTest 100% 통과 화면
- 장비 연결 사진