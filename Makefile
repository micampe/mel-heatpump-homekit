GIT_DESCRIBE := $(shell git describe --match 'v*' --dirty='-x' --always --abbrev=4)

all:
	platformio run

upload-serial:
	platformio run --environment serial --target upload --target monitor

upload-release:
	platformio run --environment release --target upload

dist:
	platformio run --environment release
	cp .pio/build/release/firmware.bin bin/mel_heatpump_$(GIT_DESCRIBE).bin

clean:
	platformio run --environment serial --environment release --target clean
