1.	GitHub에서 복사한 파일과 폴더를 /home/username/klipper 경로에 복사
2.	cd ~/klipper 명령어로 klipper 디렉토리로 이동
3.	make clean으로 이전 빌드 결과 정리
4.	make 명령어로 펌웨어 빌드 수행
5.	빌드가 완료되면 ~/klipper/out 폴더에 klipper.bin 파일이 생성됨
6.	grep '#define FW_VERSION' src/stm32/fw_version.h 명령어로 현재 MCU 펌웨어 버전을 확인할 수 있음
7.	예시 출력: #define FW_VERSION  "0.9.1"

