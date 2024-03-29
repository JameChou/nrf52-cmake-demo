cmake_minimum_required(VERSION 3.6)

# check if all the necessary tools paths have been provided.
if (NOT NRF5_SDK_PATH)
    message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif ()

if (NOT NRFJPROG)
    message(FATAL_ERROR "The path to the nrfjprog utility (NRFJPROG) must be set.")
endif ()

if (NOT ARM_NONE_EABI_TOOLCHAIN_PATH)
    message(FATAL_ERROR "arm none eabi toolchain not found.")
endif ()

# convert toolchain path to bin path
if(DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
    set(ARM_NONE_EABI_TOOLCHAIN_BIN_PATH ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin)
endif()

# check if the nRF target has been set
if (NRF_TARGET MATCHES "nrf51")

elseif (NRF_TARGET MATCHES "nrf52")

elseif (NOT NRF_TARGET)
    message(FATAL_ERROR "nRF target must be defined")
else ()
    message(FATAL_ERROR "Only nRF51 and rRF52 boards are supported right now")
endif ()

# must be set in file (not macro) scope (in macro would point to parent CMake directory)
set(DIR_OF_nRF5x_CMAKE ${CMAKE_CURRENT_LIST_DIR})
#message(STATUS "dir nRF5: ${CMAKE_CURRENT_LIST_DIR}")

macro(nRF5x_setup)
    if(NOT DEFINED ARM_GCC_TOOLCHAIN)
        message(FATAL_ERROR "The toolchain must be set up before calling this macro")
    endif()
    # fix on macOS: prevent cmake from adding implicit parameters to Xcode
    set(CMAKE_OSX_SYSROOT "/")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "")

    # language standard/version settings
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)

    # CPU specyfic settings
    if (NRF_TARGET MATCHES "nrf51")
        # nRF51 (nRF51-DK => PCA10028)
        if(NOT DEFINED NRF5_LINKER_SCRIPT)
            set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf51.ld")
        endif()

        set(CPU_FLAGS "-mcpu=cortex-m0 -mfloat-abi=soft")
        add_definitions(-DBOARD_PCA10028 -DNRF51 -DNRF51422)
        add_definitions(-DSOFTDEVICE_PRESENT -DS130 -DNRF_SD_BLE_API_VERSION=2 -DSWI_DISABLE0 -DBLE_STACK_SUPPORT_REQD)
        include_directories("${NRF5_SDK_PATH}/components/softdevice")
        list(APPEND SDK_SOURCE_FILES
                "${NRF5_SDK_PATH}/modules/nrfx/mdk/system_nrf51.c"
                "${NRF5_SDK_PATH}/modules/nrfx/mdk/gcc_startup_nrf51.S"
                )
        set(SOFTDEVICE_PATH "${NRF5_SDK_PATH}/components/softdevice/s130/hex/s130_nrf51_2.0.0_softdevice.hex")
    elseif (NRF_TARGET MATCHES "nrf52")
        # nRF52 (nRF52-DK => PCA10040)

        if(NOT DEFINED NRF5_LINKER_SCRIPT)
            set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/config/gcc_nrf52.ld")
        endif()
        set(CPU_FLAGS "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
        add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
        add_definitions(-DSOFTDEVICE_PRESENT -DS132 -DSWI_DISABLE0 -DBLE_STACK_SUPPORT_REQD -DNRF_SD_BLE_API_VERSION=6)
        include_directories(
                "${NRF5_SDK_PATH}/components/softdevice/s132/headers"
                "${NRF5_SDK_PATH}/components/softdevice/s132/headers/nrf52"
        )
        list(APPEND SDK_SOURCE_FILES
                "${NRF5_SDK_PATH}/modules/nrfx/mdk/system_nrf52.c"
                "${NRF5_SDK_PATH}/modules/nrfx/mdk/gcc_startup_nrf52.S"
                )
        set(SOFTDEVICE_PATH "${CMAKE_SOURCE_DIR}/softdevice/s132_nrf52_7.2.0_softdevice.hex")
    endif ()

    set(COMMON_FLAGS "-MP -MD -mthumb -mabi=aapcs -Wall -g3 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin --short-enums ${CPU_FLAGS}")

    # compiler/assambler/linker flags
    set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  -g3")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS "${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_ASM_FLAGS "-MP -MD -std=c99 -x assembler-with-cpp")
    set(CMAKE_EXE_LINKER_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} ${CPU_FLAGS} -Wl,--gc-sections --specs=nano.specs -lc -lnosys -lm")
    # note: we must override the default cmake linker flags so that CMAKE_C_FLAGS are not added implicitly
    set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <LINK_FLAGS> <OBJECTS> -lstdc++ -o <TARGET> <LINK_LIBRARIES>")

    # basic board definitions and drivers     
    include_directories(
            "${NRF5_SDK_PATH}/components"
            "${NRF5_SDK_PATH}/components/boards"
            "${NRF5_SDK_PATH}/components/softdevice/common"
            "${NRF5_SDK_PATH}/integration/nrfx"
            "${NRF5_SDK_PATH}/integration/nrfx/legacy"
            "${NRF5_SDK_PATH}/modules/nrfx"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/include"
            "${NRF5_SDK_PATH}/modules/nrfx/hal"
            "${NRF5_SDK_PATH}/modules/nrfx/mdk"
    )


    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/boards/boards.c"
            "${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh.c"
            "${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh_soc.c"
#            "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.c"
            "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_uart.c"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_clock.c"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_gpiote.c"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_uart.c"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_uarte.c"
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/prs/nrfx_prs.c"
            "${NRF5_SDK_PATH}/modules/nrfx/soc/nrfx_atomic.c"
            )


    # toolchain specific
    include_directories(
            "${NRF5_SDK_PATH}/components/toolchain/cmsis/include"
    )


    # libraries
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/atomic"
            "${NRF5_SDK_PATH}/components/libraries/atomic_fifo"
            "${NRF5_SDK_PATH}/components/libraries/atomic_flags"
            "${NRF5_SDK_PATH}/components/libraries/balloc"
            "${NRF5_SDK_PATH}/components/libraries/bootloader/ble_dfu"
            "${NRF5_SDK_PATH}/components/libraries/cli"
            "${NRF5_SDK_PATH}/components/libraries/crc16"
            "${NRF5_SDK_PATH}/components/libraries/crc32"
            "${NRF5_SDK_PATH}/components/libraries/crypto"
            "${NRF5_SDK_PATH}/components/libraries/csense"
            "${NRF5_SDK_PATH}/components/libraries/csense_drv"
            "${NRF5_SDK_PATH}/components/libraries/delay"
            "${NRF5_SDK_PATH}/components/libraries/ecc"
            "${NRF5_SDK_PATH}/components/libraries/experimental_section_vars"
            "${NRF5_SDK_PATH}/components/libraries/experimental_task_manager"
            "${NRF5_SDK_PATH}/components/libraries/fds"
            "${NRF5_SDK_PATH}/components/libraries/fstorage"
            "${NRF5_SDK_PATH}/components/libraries/gfx"
            "${NRF5_SDK_PATH}/components/libraries/gpiote"
            "${NRF5_SDK_PATH}/components/libraries/hardfault"
            "${NRF5_SDK_PATH}/components/libraries/hci"
            "${NRF5_SDK_PATH}/components/libraries/led_softblink"
            "${NRF5_SDK_PATH}/components/libraries/log"
            "${NRF5_SDK_PATH}/components/libraries/log/src"
            "${NRF5_SDK_PATH}/components/libraries/low_power_pwm"
            "${NRF5_SDK_PATH}/components/libraries/mem_manager"
            "${NRF5_SDK_PATH}/components/libraries/memobj"
            "${NRF5_SDK_PATH}/components/libraries/mpu"
            "${NRF5_SDK_PATH}/components/libraries/mutex"
            "${NRF5_SDK_PATH}/components/libraries/pwm"
            "${NRF5_SDK_PATH}/components/libraries/pwr_mgmt"
            "${NRF5_SDK_PATH}/components/libraries/queue"
            "${NRF5_SDK_PATH}/components/libraries/ringbuf"
            "${NRF5_SDK_PATH}/components/libraries/scheduler"
            "${NRF5_SDK_PATH}/components/libraries/sdcard"
            "${NRF5_SDK_PATH}/components/libraries/slip"
            "${NRF5_SDK_PATH}/components/libraries/sortlist"
            "${NRF5_SDK_PATH}/components/libraries/spi_mngr"
            "${NRF5_SDK_PATH}/components/libraries/stack_guard"
            "${NRF5_SDK_PATH}/components/libraries/strerror"
            "${NRF5_SDK_PATH}/components/libraries/svc"
            "${NRF5_SDK_PATH}/components/libraries/timer"
            "${NRF5_SDK_PATH}/components/libraries/twi_mngr"
            "${NRF5_SDK_PATH}/components/libraries/twi_sensor"
            "${NRF5_SDK_PATH}/components/libraries/usbd"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/audio"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc/acm"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/generic"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/kbd"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/mouse"
            "${NRF5_SDK_PATH}/components/libraries/usbd/class/msc"
            "${NRF5_SDK_PATH}/components/libraries/util"
         
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/atomic/nrf_atomic.c"
            "${NRF5_SDK_PATH}/components/libraries/atomic_fifo/nrf_atfifo.c"
            "${NRF5_SDK_PATH}/components/libraries/atomic_flags/nrf_atflags.c"
            "${NRF5_SDK_PATH}/components/libraries/balloc/nrf_balloc.c"
            "${NRF5_SDK_PATH}/components/libraries/experimental_section_vars/nrf_section_iter.c"
            "${NRF5_SDK_PATH}/components/libraries/hardfault/hardfault_implementation.c"
            "${NRF5_SDK_PATH}/components/libraries/util/nrf_assert.c"
            "${NRF5_SDK_PATH}/components/libraries/util/app_error.c"
            "${NRF5_SDK_PATH}/components/libraries/util/app_error_weak.c"
            "${NRF5_SDK_PATH}/components/libraries/util/app_error_handler_gcc.c"
            "${NRF5_SDK_PATH}/components/libraries/util/app_util_platform.c"
            "${NRF5_SDK_PATH}/components/libraries/util/sdk_mapped_flags.c"
            "${NRF5_SDK_PATH}/components/libraries/queue/nrf_queue.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_flash.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_rtt.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_serial.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_uart.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_default_backends.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_frontend.c"
            "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_str_formatter.c"
            "${NRF5_SDK_PATH}/components/libraries/memobj/nrf_memobj.c"
            "${NRF5_SDK_PATH}/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c"
            "${NRF5_SDK_PATH}/components/libraries/ringbuf/nrf_ringbuf.c"
            "${NRF5_SDK_PATH}/components/libraries/strerror/nrf_strerror.c"
            "${NRF5_SDK_PATH}/components/libraries/uart/retarget.c"

          
            )

    # Segger RTT
    include_directories(
            "${NRF5_SDK_PATH}/external/segger_rtt/"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c"
            "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT.c"
            "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_printf.c"
            )


    # Other external
    include_directories(
            "${NRF5_SDK_PATH}/external/fprintf/"
            "${NRF5_SDK_PATH}/external/utf_converter/"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/external/utf_converter/utf.c"
            "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf.c"
            "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf_format.c"
            )

        # Common Bluetooth Low Energy files
   include_directories(
           "${NRF5_SDK_PATH}/components/ble"
           "${NRF5_SDK_PATH}/components/ble/common"
           "${NRF5_SDK_PATH}/components/ble/ble_advertising"
           "${NRF5_SDK_PATH}/components/ble/ble_dtm"
           "${NRF5_SDK_PATH}/components/ble/ble_link_ctx_manager"
           "${NRF5_SDK_PATH}/components/ble/ble_racp"
           "${NRF5_SDK_PATH}/components/ble/nrf_ble_qwr"
           "${NRF5_SDK_PATH}/components/ble/peer_manager"
   )

   list(APPEND SDK_SOURCE_FILES
           "${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh_ble.c"
           "${NRF5_SDK_PATH}/components/ble/common/ble_advdata.c"
           "${NRF5_SDK_PATH}/components/ble/common/ble_conn_params.c"
           "${NRF5_SDK_PATH}/components/ble/common/ble_conn_state.c"
           "${NRF5_SDK_PATH}/components/ble/common/ble_srv_common.c"
           "${NRF5_SDK_PATH}/components/ble/ble_advertising/ble_advertising.c"
           "${NRF5_SDK_PATH}/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c"
           "${NRF5_SDK_PATH}/components/ble/ble_services/ble_nus/ble_nus.c"
           "${NRF5_SDK_PATH}/components/ble/nrf_ble_qwr/nrf_ble_qwr.c"
           )

   message(STATUS "Including Common files")  

endmacro(nRF5x_setup)

# adds a target for comiling and flashing an executable
macro(nRF5x_addExecutable EXECUTABLE_NAME SOURCE_FILES)
    # executable
    add_executable(${EXECUTABLE_NAME} ${SDK_SOURCE_FILES} ${SOURCE_FILES})
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".out")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-Wl,-Map=${EXECUTABLE_NAME}.map")

endmacro()



# adds app-level scheduler library
macro(nRF5x_addAppScheduler)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/scheduler"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/scheduler/app_scheduler.c"
            )

    message(STATUS "Including App Scheduler") 

endmacro(nRF5x_addAppScheduler)

# adds app-level FIFO libraries
macro(nRF5x_addAppFIFO)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/fifo"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/fifo/app_fifo.c"
            )

    message(STATUS "Including App Fifo") 

endmacro(nRF5x_addAppFIFO)

# adds app-level Timer libraries
macro(nRF5x_addNrfxTimer)
    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_timer.c"
            )
    message(STATUS "Including nrfx Timer") 
endmacro(nRF5x_addNrfxTimer)

# adds nrfx Timer libraries
macro(nRF5x_addAppTimer)
    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/timer/app_timer.c"
            )
    message(STATUS "Including App Timer") 
endmacro(nRF5x_addAppTimer)

# adds app-level UART libraries
macro(nRF5x_addAppUART)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/uart"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/uart/app_uart_fifo.c"
            )

    message(STATUS "Including App Uart") 

endmacro(nRF5x_addAppUART)

# adds app-level Button library
macro(nRF5x_addAppButton)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/button"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/button/app_button.c"
            )

    message(STATUS "Including App Button") 

endmacro(nRF5x_addAppButton)

# adds BSP (board support package) library
macro(nRF5x_addBSP WITH_BLE_BTN WITH_ANT_BTN WITH_NFC)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/bsp"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/bsp/bsp.c"
            )

    if (${WITH_BLE_BTN})
        list(APPEND SDK_SOURCE_FILES
                "${NRF5_SDK_PATH}/components/libraries/bsp/bsp_btn_ble.c"
                )
    endif ()

    if (${WITH_ANT_BTN})
        list(APPEND SDK_SOURCE_FILES
                "${NRF5_SDK_PATH}/components/libraries/bsp/bsp_btn_ant.c"
                )
    endif ()

    if (${WITH_NFC})
        list(APPEND SDK_SOURCE_FILES
                "${NRF5_SDK_PATH}/components/libraries/bsp/bsp_nfc.c"
                )
    endif ()

    message(STATUS "Including BSP") 

endmacro(nRF5x_addBSP)

# adds Bluetooth Low Energy GATT support library
macro(nRF5x_addBLEGATT)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_gatt"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_gatt/nrf_ble_gatt.c"
            )

    message(STATUS "Including BLE Gatt") 

endmacro(nRF5x_addBLEGATT)

# adds Bluetooth Low Energy advertising support library
macro(nRF5x_addBLEAdvertising)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/ble_advertising"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/ble_advertising/ble_advertising.c"
            )

    message(STATUS "Including BLE Advertasing") 

endmacro(nRF5x_addBLEAdvertising)

# adds Bluetooth Low Energy advertising support library
macro(nRF5x_addBLEPeerManager)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/peer_manager"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/peer_manager/auth_status_tracker.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/gatt_cache_manager.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/gatts_cache_manager.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/id_manager.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/nrf_ble_lesc.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/peer_data_storage.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/peer_database.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/peer_id.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/peer_manager.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/peer_manager_handler.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/pm_buffer.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/security_dispatcher.c"
            "${NRF5_SDK_PATH}/components/ble/peer_manager/security_manager.c"
    )

    message(STATUS "Including BLE Peer Manager") 

endmacro(nRF5x_addBLEPeerManager)

# adds app-level FDS (flash data storage) library
macro(nRF5x_addAppFDS)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/fds"
            "${NRF5_SDK_PATH}/components/libraries/fstorage"
            "${NRF5_SDK_PATH}/components/libraries/experimental_section_vars"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/fds/fds.c"
            "${NRF5_SDK_PATH}/components/libraries/fstorage/nrf_fstorage.c"
            "${NRF5_SDK_PATH}/components/libraries/fstorage/nrf_fstorage_sd.c"
            "${NRF5_SDK_PATH}/components/libraries/fstorage/nrf_fstorage_nvmc.c"
    )

    message(STATUS "Including App FDS") 

endmacro(nRF5x_addAppFDS)

# adds NFC library
macro(nRF5x_addNFC)
    # NFC includes
    include_directories(
            "${NRF5_SDK_PATH}/components/nfc/ndef/conn_hand_parser"
            "${NRF5_SDK_PATH}/components/nfc/ndef/conn_hand_parser/ac_rec_parser"
            "${NRF5_SDK_PATH}/components/nfc/ndef/conn_hand_parser/ble_oob_advdata_parser"
            "${NRF5_SDK_PATH}/components/nfc/ndef/conn_hand_parser/le_oob_rec_parser"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/ac_rec"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/ble_oob_advdata"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/ble_pair_lib"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/ble_pair_msg"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/common"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/ep_oob_rec"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/hs_rec"
            "${NRF5_SDK_PATH}/components/nfc/ndef/connection_handover/le_oob_rec"
            "${NRF5_SDK_PATH}/components/nfc/ndef/generic/message"
            "${NRF5_SDK_PATH}/components/nfc/ndef/generic/record"
            "${NRF5_SDK_PATH}/components/nfc/ndef/launchapp"
            "${NRF5_SDK_PATH}/components/nfc/ndef/parser/message"
            "${NRF5_SDK_PATH}/components/nfc/ndef/parser/record"
            "${NRF5_SDK_PATH}/components/nfc/ndef/text"
            "${NRF5_SDK_PATH}/components/nfc/ndef/uri"
            "${NRF5_SDK_PATH}/components/nfc/t2t_lib"
            "${NRF5_SDK_PATH}/components/nfc/t2t_parser"
            "${NRF5_SDK_PATH}/components/nfc/t4t_lib"
            "${NRF5_SDK_PATH}/components/nfc/t4t_parser/apdu"
            "${NRF5_SDK_PATH}/components/nfc/t4t_parser/cc_file"
            "${NRF5_SDK_PATH}/components/nfc/t4t_parser/hl_detection_procedure"
            "${NRF5_SDK_PATH}/components/nfc/t4t_parser/tlv"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/nfc"
            )

    message(STATUS "Including NFC") 

endmacro(nRF5x_addNFC)

macro(nRF5x_addSensorSim)
    include_directories(
            "${NRF5_SDK_PATH}/components/libraries/sensorsim"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/libraries/sensorsim/sensorsim.c" 
            )

    message(STATUS "Including SensorSim") 

endmacro(nRF5x_addSensorSim)

macro(nRF5x_addBLEDiscovery)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/ble_db_discovery"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/ble_db_discovery/ble_db_discovery.c"
            )
    message(STATUS "Including BLE Discovery") 

endmacro(nRF5x_addBLEDiscovery)

macro(nRF5x_addBLEgq)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_gq"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_gq/nrf_ble_gq.c"
            )

    message(STATUS "Including BLE gq") 

endmacro(nRF5x_addBLEgq)

macro(nRF5x_addBLEScan)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_scan"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/nrf_ble_scan/nrf_ble_scan.c" 
            )

    message(STATUS "Including BLE Scan")    

endmacro(nRF5x_addBLEScan)

macro(nRF5x_addBLEService NAME)
    include_directories(
            "${NRF5_SDK_PATH}/components/ble/ble_services/${NAME}"
    )

    list(APPEND SDK_SOURCE_FILES
            "${NRF5_SDK_PATH}/components/ble/ble_services/${NAME}/${NAME}.c"
            )

    message(STATUS "Including BLE Service: ${NAME}") 

    if(${NAME} STREQUAL "ble_nus")
    	message(STATUS "Including BLE Service: ${NAME}")
    	message(STATUS "Setting  NRF_LOG_BACKEND_RTT_ENABLED 	0")
    	message(STATUS "Setting  BLE_NUS_ENABLED 			1    ")
    	message(STATUS "Setting  BLE_NUS_CONFIG_LOG_ENABLED		1") 	            
    endif()

endmacro(nRF5x_addBLEService)

macro(nRF5x_addNrfxPPI)
    list(APPEND SDK_SOURCE_FILES
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_ppi.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_ppi.c"
    )
    message(STATUS "Including nrfx ppi") 
endmacro(nRF5x_addNrfxPPI)

macro(nRF5x_addNvmc)
    list(APPEND SDK_SOURCE_FILES
        "${NRF5_SDK_PATH}/modules/nrfx/hal/nrf_nvmc.c"
    )
    message(STATUS "Including nrf nvmc hal") 
endmacro(nRF5x_addNvmc)

macro(nRF5x_addWDT)
    list(APPEND SDK_SOURCE_FILES
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_wdt.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.c"
    )
endmacro(nRF5x_addWDT)

macro(nRF5x_addSAADC)
    list(APPEND SDK_SOURCE_FILES
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_saadc.c"
    )
endmacro(nRF5x_addSAADC)

macro(nRF5x_addTWI)
    list(APPEND SDK_SOURCE_FILES
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_twi.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_twi.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_twim.c"
    )
endmacro(nRF5x_addTWI)