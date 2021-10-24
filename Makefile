FREERTOS_VER ?= V10.4.5

.PHONY: barectf

all: check barectf
	make -C Demo
	make -C rvsim

run: all
	make -C Demo run

check:
	[ -d FreeRTOS-Kernel ] || git clone -b ${FREERTOS_VER} https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel

barectf:
	@mkdir -p barectf
	cd config && barectf generate --code-dir ../barectf \
        --headers-dir ../include --metadata-dir ../barectf config.yaml && cd ..

clean:
	make -C Demo clean
	make -C rvsim clean

distclean:
	rm -rf FreeRTOS-Kernel barectf

