※ printer.cfg 파일에 매크로 항목을 추가하고, G-code로 호출해야 합니다.

1. [mcu_flash_request]
mflashwrite flag=x
-> G-code로 mflashwrite flag=1 또는 mflashwrite flag=0을 입력하면, 해당 값을 MCU Flash에 저장합니다.
mflashread
-> G-code로 mflashread를 입력하면, MCU Flash에 저장된 데이터를 읽어와 반환합니다.

2. [mcu_fw_version_request]
mcufwread
-> G-code로 mcufwread를 입력하면 현재 MCU 버전을 문자열로 반환합니다.
