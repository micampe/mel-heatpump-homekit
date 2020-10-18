GIT_DESCRIBE := $(shell git describe --match 'v*' --dirty='-x' --always --abbrev=4)

all:
	platformio --caller vim run --silent --environment serial

upload-serial:
	platformio --caller vim run --silent --environment serial --target upload --target monitor

upload-ota:
	platformio --caller vim run --silent --environment ota --target upload

dist:	all
	cp .pio/build/serial/firmware.bin bin/mel_heatpump_$(GIT_DESCRIBE).bin
	gzip --force bin/mel_heatpump_$(GIT_DESCRIBE).bin

clean:
	platformio --caller vim run -t clean
