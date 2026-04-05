import hashlib
import os
import subprocess
import opendbc
from SCons.Script import *

# 1. ENVIRONMENT SETUP
opendbc_path = opendbc.INCLUDE_PATH

def get_version():
  try:
    return subprocess.check_output(["git", "describe", "--always", "--dirty"]).strip().decode('utf8')
  except:
    return "unknown"

version = get_version()

# 2. SOURCE DEFINITIONS
system_srcs = [
  "./board/main_definitions.c", "./board/utils.c", "./board/libc.c",
  "./board/crc.c", "./board/gitversion.c", "./board/safety_definitions.c",
  "./board/sys/critical.c", "./board/sys/faults.c", "./board/early_init.c",
  "./board/provision.c",
]

driver_srcs = [
  "./board/drivers/can_common.c", "./board/drivers/gpio.c", "./board/drivers/led.c",
  "./board/drivers/pwm.c", "./board/drivers/simple_watchdog.c", "./board/drivers/uart.c",
  "./board/drivers/usb.c", "./board/drivers/spi.c", "./board/drivers/timers.c",
  "./board/drivers/fdcan.c", "./board/drivers/registers.c", "./board/drivers/interrupts.c",
]

app_srcs = [
  "./board/can_comms.c", "./board/main_comms.c", "./board/safety_mode_wrapper.c",
  "./board/sys/power_saving.c",
]

ll_srcs = [
  "./board/stm32h7/lluart.c", "./board/stm32h7/llusb.c", "./board/stm32h7/llspi.c",
  "./board/stm32h7/clock.c", "./board/stm32h7/llfdcan.c", "./board/stm32h7/peripherals.c",
  "./board/stm32h7/interrupt_handlers.c", "./board/stm32h7/lladc.c",
]

# Versioning logic for health packets
def version_hash(path):
  with open(path, "rb") as f:
    return int.from_bytes(hashlib.sha256(f.read()).digest()[:4], 'little')

hh = version_hash("board/health.h")
ch = version_hash(os.path.join(opendbc_path, "opendbc/safety/can.h"))
jh = version_hash("board/jungle/jungle_health.h")

common_flags = [
  f"-DHEALTH_PACKET_VERSION=0x{hh:08X}U",
  f"-DCAN_PACKET_VERSION_HASH=0x{ch:08X}U",
  f"-DJUNGLE_HEALTH_PACKET_VERSION=0x{jh:08X}U",
]

def build_project(project_name, main_file, project_srcs, extra_flags=[]):
  project_dir = Dir(f'./board/obj/{project_name}/')
  
  flags = [
    "-mcpu=cortex-m7", "-mhard-float", "-mfpu=fpv5-d16",
    "-DSTM32H7", "-DSTM32H725xx",
    "-Wall", "-Wextra", "-Wstrict-prototypes", "-Werror",
    "-mlittle-endian", "-mthumb", "-nostdlib", "-fno-builtin",
    "-std=gnu11", "-fmax-errors=1", "-fsingle-precision-constant",
    "-Os", "-g", f"-T./board/stm32h7/stm32h7x5_flash.ld",
    "-Iboard/stm32h7/inc", "-I.", f"-I{opendbc_path}", "-Iboard/obj"
  ] + extra_flags + common_flags

  env = Environment(
    ENV=os.environ,
    CC='arm-none-eabi-gcc', AS='arm-none-eabi-gcc',
    OBJCOPY='arm-none-eabi-objcopy', OBJDUMP='arm-none-eabi-objdump',
    CFLAGS=flags, LINKFLAGS=flags,
    tools=["default", "compilation_db"],
  )

  # 3. GENERATE HEADERS (Within target env to track dependencies)
  gitversion_h = env.Textfile(target="board/obj/gitversion.h", source=[f'extern const uint8_t gitversion[{len(version)+1}];\n'])
  gitversion_c = env.Textfile(target="board/gitversion.c", source=[f'#include <stdint.h>\nconst uint8_t gitversion[{len(version)+1}] = "{version}";\n'])
  
  cert_h_content = [
    '#include <stdint.h>',
    '#include "board/crypto/rsa.h"',
    'extern const RSAPublicKey debug_rsa_key;',
    'extern const RSAPublicKey release_rsa_key;'
  ]
  cert_h = env.Textfile(target="board/obj/cert.h", source=cert_h_content)

  def make_objs(srcs, environment, suffix=""):
    objs = []
    for s in srcs:
      name = s.replace("./", "").replace("/", "_").replace(".c", suffix + ".o").replace(".s", suffix + ".o")
      obj_path = os.path.join(project_dir.path, name)
      obj = environment.Object(obj_path, s)
      # Ensure object waits for headers
      environment.Depends(obj, [gitversion_h, cert_h])
      objs.append(obj)
    return objs

  startup = "./board/stm32h7/startup_stm32h7x5xx.s"
  
  # Bootstub
  bs_srcs = ["./board/crypto/rsa.c", "./board/crypto/sha.c", "./board/bootstub.c", "./board/flasher.c", "./board/stm32h7/llflash.c", "./board/bootstub_declarations.c"] + system_srcs + driver_srcs + ll_srcs + project_srcs
  bs_env = env.Clone()
  bs_env.Append(CFLAGS=["-DBOOTSTUB"])
  bs_objs = make_objs(bs_srcs, bs_env, suffix=".bootstub") + make_objs([startup], bs_env, suffix=".bootstub")
  bs_elf = bs_env.Program(f"{project_dir}/bootstub.elf", bs_objs)
  env.Command(f"./board/obj/bootstub.{project_name}.bin", bs_elf, "arm-none-eabi-objcopy -O binary $SOURCE $TARGET")

  # App
  app_full_srcs = [main_file] + system_srcs + driver_srcs + app_srcs + ll_srcs + project_srcs
  app_objs = make_objs(app_full_srcs, env, suffix=".app") + make_objs([startup], env, suffix=".app")
  app_link_flags = ["-Wl,--section-start,.isr_vector=0x8020000"]
  app_elf = env.Program(f"{project_dir}/main.elf", app_objs, LINKFLAGS=env['LINKFLAGS'] + app_link_flags)
  app_bin = env.Command(f"{project_dir}/main.bin", app_elf, "arm-none-eabi-objcopy -O binary $SOURCE $TARGET")
  
  # Sign
  cert_fn = "board/certs/debug"
  env.Command(f"./board/obj/{project_name}.bin.signed", app_bin, f"SETLEN=1 python3 board/crypto/sign.py $SOURCE $TARGET {cert_fn}")

# --- Target Instances ---
panda_board_srcs = ["./board/stm32h7/board.c", "./board/boards/red.c", "./board/boards/tres.c", "./board/boards/cuatro.c", "./board/boards/unused_funcs.c", "./board/drivers/fan.c", "./board/drivers/bootkick.c", "./board/drivers/harness.c", "./board/drivers/fake_siren.c", "./board/stm32h7/sound.c", "./board/drivers/clock_source.c", "./board/stm32h7/llfan.c"]
build_project("panda_h7", "./board/main.c", panda_board_srcs, ["-DALLOW_DEBUG"])

jungle_board_srcs = ["board/jungle/boards/main_definitions.c", "./board/jungle/boards/board_v2.c", "./board/jungle/stm32h7/board.c"]
build_project("panda_jungle_h7", "./board/jungle/main.c", jungle_board_srcs, ["-DPANDA_JUNGLE", "-DALLOW_DEBUG"])

body_board_srcs = ["board/body/main_definitions.c", "./board/body/motor_encoder.c", "./board/body/motor_control.c", "./board/body/boards/board_body.c", "./board/body/stm32h7/board.c"]
build_project("body_h7", "./board/body/main.c", body_board_srcs, ["-DPANDA_BODY", "-DALLOW_DEBUG"])

SConscript('tests/libpanda/SConscript')
