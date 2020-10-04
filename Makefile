GIT_DESCRIBE := $(shell git describe --match 'v*' --dirty='-x' --always)

all:
	platformio --caller vim run --silent

upload-serial:
	platformio --caller vim run --silent --target upload

upload-ota:
	platformio --caller vim run --silent --environment ota --target upload

upload-prod:
	platformio --caller vim run --silent --environment prod --target upload

dist:
	platformio --caller vim run --silent --environment prod
	cp .pio/build/prod/firmware.bin bin/mie-heatpump_$(GIT_DESCRIBE).bin

clean:
	platformio --caller vim run -t clean
