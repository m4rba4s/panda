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
  "./board/provision.c", "./board/certs/certs.c"
]

# Base drivers shared by all targets
base_driver_srcs = [
  "./board/drivers/gpio.c", "./board/drivers/led.c",
  "./board/drivers/pwm.c", "./board/drivers/simple_watchdog.c", "./board/drivers/uart.c",
  "./board/drivers/usb.c", "./board/drivers/spi.c", "./board/drivers/timers.c",
  "./board/drivers/registers.c", "./board/drivers/interrupts.c",
]

# Panda specific drivers
panda_driver_srcs = [
  "./board/drivers/can_common.c", "./board/drivers/fdcan.c",
  "./board/drivers/fan.c", "./board/drivers/bootkick.c",
  "./board/drivers/harness.c", "./board/drivers/fake_siren.c",
  "./board/drivers/clock_source.c",
]

# Common app logic
common_app_srcs = [
  "./board/can_comms.c", "./board/safety_mode_wrapper.c",
  "./board/sys/power_saving.c",
]

# Shared debug callback (Panda and Jungle)
shared_debug_srcs = ["./board/debug_callback.c"]

# Panda specific app logic
panda_app_srcs = ["./board/main_comms.c"]

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

# --- GLOBAL FILE GENERATION ---
env_gen = Environment(ENV=os.environ)
gitversion_h = env_gen.Textfile(target="board/obj/gitversion.h", source=[f'extern const uint8_t gitversion[{len(version)+1}];\n'])
gitversion_c = env_gen.Textfile(target="board/gitversion.c", source=[f'#include <stdint.h>\nconst uint8_t gitversion[{len(version)+1}] = "{version}";\n'])

cert_h_content = [
  '#include <stdint.h>',
  '#include "board/crypto/rsa.h"',
  'extern const RSAPublicKey debug_rsa_key;',
  'extern const RSAPublicKey release_rsa_key;'
]
cert_h = env_gen.Textfile(target="board/obj/cert.h", source=cert_h_content)

generated_headers = [gitversion_h, cert_h]

def build_project(project_name, main_file, project_srcs, extra_flags=[], is_panda=True, use_shared_debug=True):
  project_dir = Dir(f'./board/obj/{project_name}/')
  
  flags = [
    "-mcpu=cortex-m7", "-mhard-float", "-mfpu=fpv5-d16",
    "-DSTM32H7", "-DSTM32H725xx",
    "-Wall", "-Wextra", "-Wstrict-prototypes", "-Werror",
    "-mlittle-endian", "-mthumb", "-nostdlib", "-fno-builtin",
    "-nostartfiles", "-nodefaultlibs",
    "-std=gnu11", "-fmax-errors=1", "-fsingle-precision-constant",
    "-Os", "-g",
    "-Iboard/stm32h7/inc", "-I.", f"-I{opendbc_path}", "-Iboard/obj"
  ] + extra_flags + common_flags

  env = Environment(
    ENV=os.environ,
    CC='arm-none-eabi-gcc', AS='arm-none-eabi-gcc',
    OBJCOPY='arm-none-eabi-objcopy', OBJDUMP='arm-none-eabi-objdump',
    CFLAGS=flags, CCFLAGS=flags,
    ASFLAGS=flags + ["-x", "assembler-with-cpp"],
    ASCOM='$AS $ASFLAGS -c -o $TARGET $SOURCE',
    tools=["default", "compilation_db"],
  )

  def make_objs(srcs, environment, suffix=""):
    objs = []
    for s in srcs:
      name = s.replace("./", "").replace("/", "_").replace(".c", suffix + ".o").replace(".s", suffix + ".o")
      obj_path = os.path.join(project_dir.path, name)
      obj = environment.Object(obj_path, s)
      environment.Depends(obj, generated_headers)
      objs.append(obj)
    return objs

  startup = "./board/stm32h7/startup_stm32h7x5xx.s"
  linker_script = "-T./board/stm32h7/stm32h7x5_flash.ld"
  
  # Bootstub
  bs_full_srcs = ["./board/crypto/rsa.c", "./board/crypto/sha.c", "./board/bootstub.c", "./board/flasher.c", "./board/stm32h7/llflash.c", "./board/bootstub_declarations.c"] + system_srcs + base_driver_srcs + panda_driver_srcs + ll_srcs + project_srcs
  if use_shared_debug:
    bs_full_srcs += shared_debug_srcs
    
  bs_env = env.Clone()
  bs_env.Append(CFLAGS=["-DBOOTSTUB"], CCFLAGS=["-DBOOTSTUB"])
  bs_objs = make_objs(bs_full_srcs, bs_env, suffix=".bootstub") + make_objs([startup], bs_env, suffix=".bootstub")
  bs_elf = bs_env.Program(f"{project_dir}/bootstub.elf", bs_objs, LINKFLAGS=env['CFLAGS'] + [linker_script])
  env.Command(f"./board/obj/bootstub.{project_name}.bin", bs_elf, "arm-none-eabi-objcopy -O binary $SOURCE $TARGET")

  # App
  app_src_list = [main_file] + system_srcs + base_driver_srcs + panda_driver_srcs + common_app_srcs + ll_srcs + project_srcs
  if is_panda:
    app_src_list += panda_app_srcs
  if use_shared_debug:
    app_src_list += shared_debug_srcs
    
  app_objs = make_objs(app_src_list, env, suffix=".app") + make_objs([startup], env, suffix=".app")
  app_link_flags = [linker_script, "-Wl,--section-start,.isr_vector=0x8020000"]
  app_elf = env.Program(f"{project_dir}/main.elf", app_objs, LINKFLAGS=env['CFLAGS'] + app_link_flags)
  app_bin = env.Command(f"{project_dir}/main.bin", app_elf, "arm-none-eabi-objcopy -O binary $SOURCE $TARGET")
  
  # Sign
  cert_fn = "board/certs/debug"
  env.Command(f"./board/obj/{project_name}.bin.signed", app_bin, f"SETLEN=1 python3 board/crypto/sign.py $SOURCE $TARGET {cert_fn}")

# --- Target Instances ---
panda_board_specific = ["./board/stm32h7/board.c", "./board/boards/red.c", "./board/boards/tres.c", "./board/boards/cuatro.c", "./board/boards/unused_funcs.c", "./board/stm32h7/sound.c", "./board/stm32h7/llfan.c"]
build_project("panda_h7", "./board/main.c", panda_board_specific, ["-DALLOW_DEBUG"], is_panda=True, use_shared_debug=True)

jungle_board_specific = ["board/jungle/boards/main_definitions.c", "./board/jungle/boards/board_v2.c", "./board/jungle/stm32h7/board.c"]
build_project("panda_jungle_h7", "./board/jungle/main.c", jungle_board_specific, ["-DPANDA_JUNGLE", "-DALLOW_DEBUG"], is_panda=False, use_shared_debug=True)

body_board_specific = ["board/body/main_definitions.c", "./board/body/motor_encoder.c", "./board/body/motor_control.c", "./board/body/boards/board_body.c", "./board/body/stm32h7/board.c"]
build_project("body_h7", "./board/body/main.c", body_board_specific, ["-DPANDA_BODY", "-DALLOW_DEBUG"], is_panda=False, use_shared_debug=False)

SConscript('tests/libpanda/SConscript', exports={'env': env_gen})
