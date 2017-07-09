cmake_minimum_required(VERSION 3.6)

set(TARGET_NAME "esp_aqua")

project(${TARGET_NAME} C CXX ASM)

set(CMAKE_VERBOSE_MAKEFILE 1)


set(SDK_DIR "${CMAKE_SOURCE_DIR}/ESP8266_RTOS_SDK")
set(SDK_LIB_DIR "${CMAKE_SOURCE_DIR}/ESP8266_RTOS_SDK/lib")
set(FATFS_DIR "${CMAKE_SOURCE_DIR}/fatfs2")
set(USER_DIR "${CMAKE_SOURCE_DIR}/user")
set(SDIO_DIR "${CMAKE_SOURCE_DIR}/sdio")
set(DRIVER_DIR "${CMAKE_SOURCE_DIR}/driver")
set(LINKER_DIR "${SDK_DIR}/ld")
set(GDBSTUB_DIR "${CMAKE_SOURCE_DIR}/gdbstub2")

set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen_bin")


set(PYTHON "/usr/bin/python")
set(GEN_APP_DIR "${SDK_DIR}/tools")
set(MODE "3")
set(FREQ_DIV "0")
set(SIZE_MAP "6")



set(GCC_PREFIX "xtensa-lx106-elf")
set(EXTENSION "")
set(TOOLCHAIN_PATH "/home/xtensa-compiler")

#set(ESP8266_ESPTOOL ${PYTHON} ${TOOLCHAIN_PATH}/bin/esptool.py)
set(ESP8266_ESPTOOL esptool)

set(CMD_GCC     "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-gcc${EXTENSION}")
set(CMD_OBJDUMP "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-objdump${EXTENSION}")
set(CMD_OBJCOPY "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-objcopy${EXTENSION}")
set(CMD_SIZE    "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-size${EXTENSION}")
set(CMAKE_AR    "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-ar${EXTENSION}")
set(CMD_NM      "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-nm${EXTENSION}")
set(CMD_G++     "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-g++${EXTENSION}")
set(CMD_LD      "${TOOLCHAIN_PATH}/bin/${GCC_PREFIX}-ld${EXTENSION}")

MESSAGE(${CMD_GCC})

set(CMAKE_C_COMPILER   ${CMD_GCC})
set(CMAKE_ASM_COMPILER ${CMD_GCC})
set(CMAKE_LINKER ${CMD_LD})

set(LWIP_FILES
        ${SDK_DIR}/third_party/lwip/api/api_lib.c
        ${SDK_DIR}/third_party/lwip/api/api_msg.c
        ${SDK_DIR}/third_party/lwip/api/err.c
        ${SDK_DIR}/third_party/lwip/api/netbuf.c
        ${SDK_DIR}/third_party/lwip/api/netdb.c
        ${SDK_DIR}/third_party/lwip/api/netifapi.c
        ${SDK_DIR}/third_party/lwip/api/sockets.c
        ${SDK_DIR}/third_party/lwip/api/tcpip.c
        ${SDK_DIR}/third_party/lwip/api/multi-threads/sockets_mt.c

        ${SDK_DIR}/third_party/lwip/apps/sntp_time.c
        ${SDK_DIR}/third_party/lwip/apps/sntp.c
        ${SDK_DIR}/third_party/lwip/apps/time.c

        ${SDK_DIR}/third_party/lwip/arch/sys_arch.c

        ${SDK_DIR}/third_party/lwip/core/def.c
        ${SDK_DIR}/third_party/lwip/core/dhcp.c
        #${SDK_DIR}/third_party/lwip/core/dhcpserver.c
        ${SDK_DIR}/third_party/lwip/core/dns.c
        ${SDK_DIR}/third_party/lwip/core/inet_chksum.c
        ${SDK_DIR}/third_party/lwip/core/init.c
        ${SDK_DIR}/third_party/lwip/core/mem.c
        ${SDK_DIR}/third_party/lwip/core/memp.c
        ${SDK_DIR}/third_party/lwip/core/netif.c
        ${SDK_DIR}/third_party/lwip/core/pbuf.c
        ${SDK_DIR}/third_party/lwip/core/raw.c
        ${SDK_DIR}/third_party/lwip/core/stats.c
        ${SDK_DIR}/third_party/lwip/core/sys.c
        ${SDK_DIR}/third_party/lwip/core/tcp.c
        ${SDK_DIR}/third_party/lwip/core/tcp_in.c
        ${SDK_DIR}/third_party/lwip/core/tcp_out.c
        ${SDK_DIR}/third_party/lwip/core/timers.c
        ${SDK_DIR}/third_party/lwip/core/udp.c

        ${SDK_DIR}/third_party/lwip/core/ipv4/autoip.c
        ${SDK_DIR}/third_party/lwip/core/ipv4/icmp.c
        ${SDK_DIR}/third_party/lwip/core/ipv4/igmp.c
        ${SDK_DIR}/third_party/lwip/core/ipv4/ip4_addr.c
        ${SDK_DIR}/third_party/lwip/core/ipv4/ip4.c
        ${SDK_DIR}/third_party/lwip/core/ipv4/ip_frag.c

        #${SDK_DIR}/third_party/lwip/core/ipv6/dhcp6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/ethip6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/icmp6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/inet6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/ip6_addr.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/ip6_frag.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/ip6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/mld6.c
        #${SDK_DIR}/third_party/lwip/core/ipv6/nd6.c


#        ${SDK_DIR}/third_party/lwip/core/snmp/asn1_dec.c
#        ${SDK_DIR}/third_party/lwip/core/snmp/asn1_enc.c
#        ${SDK_DIR}/third_party/lwip/core/snmp/mib2.c
#        ${SDK_DIR}/third_party/lwip/core/snmp/mib_structs.c
#        ${SDK_DIR}/third_party/lwip/core/snmp/msg_in.c
#        ${SDK_DIR}/third_party/lwip/core/snmp/msg_out.c

        ${SDK_DIR}/third_party/lwip/netif/etharp.c
        ${SDK_DIR}/third_party/lwip/netif/ethernetif.c
        ${SDK_DIR}/third_party/lwip/netif/slipif.c

        ${SDK_DIR}/third_party/lwip/netif/ppp/auth.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/chap.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/chpms.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/fsm.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/ipcp.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/lcp.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/magic.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/md5.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/pap.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/ppp.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/ppp_oe.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/randm.c
        ${SDK_DIR}/third_party/lwip/netif/ppp/vj.c
)


set(FREE_RTOS_FILES
        ${SDK_DIR}/third_party/freertos/croutine.c
        ${SDK_DIR}/third_party/freertos/heap_4.c
        ${SDK_DIR}/third_party/freertos/list.c
        ${SDK_DIR}/third_party/freertos/port.c
        ${SDK_DIR}/third_party/freertos/queue.c
        ${SDK_DIR}/third_party/freertos/tasks.c
        ${SDK_DIR}/third_party/freertos/timers.c
)

set(GDBSTUB_FILES
        ${GDBSTUB_DIR}/gdbstub.c
        ${GDBSTUB_DIR}/gdbstub-entry.S
)




include_directories(
        ${FATFS_DIR}
        ${DRIVER_DIR}
        ${USER_DIR}
        ${SDIO_DIR}
        ${GDBSTUB_DIR}
        ${SDK_DIR}/include
        ${SDK_DIR}/include/espressif
        ${SDK_DIR}/include/espressif/esp8266
        ${SDK_DIR}/include/freertos
        ${SDK_DIR}/include/lwip
        ${SDK_DIR}/include/lwip/apps
        ${SDK_DIR}/include/lwip/arch
        ${SDK_DIR}/include/lwip/ipv4
        ${SDK_DIR}/include/lwip/ipv4/lwip
        ${SDK_DIR}/include/lwip/ipv6
        ${SDK_DIR}/include/lwip/ipv6/lwip
        ${SDK_DIR}/include/lwip/lwip
        ${SDK_DIR}/include/lwip/lwip/multi-threads
        ${SDK_DIR}/include/lwip/netif
        ${SDK_DIR}/include/lwip/posix
        ${SDK_DIR}/include/lwip/posix/sys
        ${SDK_DIR}/include/lwip
        ${SDK_DIR}/extra_include
        ${SDK_DIR}/extra_include/xtensa
        ${SDK_DIR}/extra_include/xtensa/config
        ${SDK_DIR}/extra_include/xtensa/tie
        ${SDK_DIR}/third_party/lwip
        ${SDK_DIR}/third_party/lwip/netif
        ${SDK_DIR}/third_party/lwip/netif/ppp

)


add_definitions(
        -DICACHE_FLASH
        -DMEMLEAK_DEBUG
)


add_compile_options(
        #-M
        -Og # Os bez gdbstub
        -g
        -Wpointer-arith
        -Wundef
        -Werror
        -Wl,-EL
        -fno-inline-functions
        -nostdlib
        -mlongcalls
        -mtext-section-literals
        -ffunction-sections
        -fdata-sections
        -fno-builtin-printf
        -fno-jump-tables
        #-ggdb
        #-fno-aggressive-loop-optimizations
)

link_libraries(
        #-Os
        -L${SDK_LIB_DIR}
        -Wl,--gc-sections
        -nostdlib
        #-T${LINKER_DIR}/eagle.app.v6.new.2048.ld
        -T${LINKER_DIR}/eagle.app.v6.ld
        -Wl,--no-check-sections
        "-u call_user_start"
        -Wl,-static
        -Wl,--start-group
        -lcirom
        -lmirom
        -lcrypto
        -lespconn
        -lespnow
        -lfreertos
        -lgcc
        -lhal
        -ljson
        -llwip
        -lmain
        -lmesh
        -lnet80211
        -lnopoll
        -lphy
        -lpp
        -lpwm
        -lsmartconfig
        -lspiffs
        -lssl
        -lwpa
        -lwps
        -Wl,--end-group

)

set_source_files_properties(
        ${LWIP_FILES} PROPERTIES
        COMPILE_FLAGS "-fno-aggressive-loop-optimizations"
        COMPILE_DEFINITIONS "_POSIX_SOURCE"
        COMPILE_DEFINITIONS "LWIP_OPEN_SRC"
        COMPILE_DEFINITIONS "PBUF_RSV_FOR_WLAN"
        COMPILE_DEFINITIONS "EBUF_LWIP"
)


add_executable(${TARGET_NAME}

        #${SDK_DIR}/driver_lib/driver/gpio.c
        #${SDK_DIR}/driver_lib/driver/uart.c
        #${SDK_DIR}/driver_lib/driver/hw_timer.c
        #${SDK_DIR}/driver_lib/driver/i2c_master.c
        #${SDK_DIR}/driver_lib/driver/spi_interface.c

        ${LWIP_FILES}
        ${FREE_RTOS_FILES}
        ${GDBSTUB_FILES}

        ${SDIO_DIR}/sdio.c

        ${USER_DIR}/access_point.c
        #${USER_DIR}/mmc_esp8266.c
        ${USER_DIR}/tcp_server.c
        ${USER_DIR}/user_main.c

        ${FATFS_DIR}/diskio.c
        ${FATFS_DIR}/ff.c
        ${FATFS_DIR}/syscall.c
        ${FATFS_DIR}/unicode.c
        ${FATFS_DIR}/volumes.c

        ${DRIVER_DIR}/gpio.c
        ${DRIVER_DIR}/spi.c
        )

# remove multiply defined printf functionality (duplicated at libmain.a
add_custom_command(TARGET ${TARGET_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_AR} -dv ${SDK_LIB_DIR}/libcirom.a lib_a-printf.o
        COMMAND rm -r -f ${OUT_DIR}
        COMMAND rm -r -f ${TARGET_NAME}
        COMMAND mkdir ${OUT_DIR}
        )

# post build objcopy
add_custom_command(TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMD_OBJCOPY} --only-section .text -O binary ${TARGET_NAME} eagle.app.v6.text.bin
        COMMAND ${CMD_OBJCOPY} --only-section .data -O binary ${TARGET_NAME} eagle.app.v6.data.bin
        COMMAND ${CMD_OBJCOPY} --only-section .rodata -O binary ${TARGET_NAME} eagle.app.v6.rodata.bin
        COMMAND ${CMD_OBJCOPY} --only-section .irom0.text -O binary ${TARGET_NAME} eagle.app.v6.irom0text.bin
        COMMAND ${CMD_OBJDUMP} -x -s ${TARGET_NAME} > ${OUT_DIR}/eagle.dump
        COMMAND ${CMD_OBJDUMP} -S ${TARGET_NAME} > ${OUT_DIR}/eagle.S

        COMMAND ${PYTHON} ${GEN_APP_DIR}/gen_appbin.py ${TARGET_NAME} 0 ${MODE} ${FREQ_DIV} ${SIZE_MAP}

        COMMAND mv eagle.app.v6.text.bin ${OUT_DIR}/
        COMMAND mv eagle.app.v6.data.bin ${OUT_DIR}/
        COMMAND mv eagle.app.v6.rodata.bin ${OUT_DIR}/
        COMMAND mv eagle.app.v6.irom0text.bin ${OUT_DIR}/

        )


add_custom_target(
        flash_app
        # COMMAND python "${GEN_APP_DIR}/gen_appbin.py ${TARGET_NAME} 0 ${MODE} ${FREQ_DIV} ${SIZE_MAP}" 576000
        COMMAND ${ESP8266_ESPTOOL} -p /dev/ttyUSB0 -b  230400 write_flash -ff 40m -fm dout -fs 4MB 0x0 ${OUT_DIR}/eagle.app.flash.bin 0x20000 ${OUT_DIR}/eagle.app.v6.irom0text.bin
        # COMMAND ${ESP8266_ESPTOOL} -p /dev/ttyUSB0 -b 230400 write_flash -ff 40m -fm dout -fs 4m 0x20000 ${CMAKE_CURRENT_BINARY_DIR}/eagle.app.v6.irom0text.bin
        # COMMAND ${ESP8266_ESPTOOL} --port /dev/ttyUSB0 write_flash -fm qio -fs 32m -ff 40m 0x00000 eagle.app.flash.bin 0x20000 eagle.app.v6.irom0text.bin
)

add_custom_target(
        flash_default
        COMMAND ${ESP8266_ESPTOOL} write_flash -ff 40m -fm dout -fs 4MB
                0x3fe000 ${SDK_DIR}/bin/blank.bin
                0x3fc000 ${SDK_DIR}/bin/esp_init_data_default.bin
                0xfe000 ${SDK_DIR}/bin/blank.bin
)

add_custom_target(
        erase_all
        COMMAND ${ESP8266_ESPTOOL} erase_flash
)


#add_custom_target(flash COMMAND ${ESP8266_ESPTOOL} -cp ${ESP8266_ESPTOOL_COM_PORT} -cf firmware_${FW_ADDR_1}.bin -ca 0x40000 -cf firmware_${FW_ADDR_2}.bin)

#
#        @mv eagle.app.flash.bin $(BIN_PATH)/eagle.flash.bin
#        @mv eagle.app.v6.irom0text.bin $(BIN_PATH)/eagle.irom0text.bin
#        @rm eagle.app.v6.*
#        @echo "BIN_PATH: $(BIN_PATH)"
#        @echo ""
#        @echo "No boot needed."
#        @echo "Generate eagle.flash.bin and eagle.irom0text.bin successully in BIN_PATH"
#        @echo "eagle.flash.bin-------->0x00000"
#        @echo "eagle.irom0text.bin---->0x20000"
