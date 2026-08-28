# CryptoMachine Seed RP2350 Production Provisioning Specification

Status: DRAFT, NO OTP WRITES AUTHORIZED

This document defines the intended RP2350 production security policy for CryptoMachine Seed. It is a planning and review artifact only. It does not authorize OTP programming.

## 1. Security goals

CryptoMachine Seed should:

- execute only CryptoMachine-approved production firmware
- retain a controlled firmware recovery path
- expose no unnecessary runtime USB, UART, filesystem, or debug interface
- avoid persistent storage of seed material
- support recovery from loss or compromise of the primary firmware-signing key
- avoid irreversible OTP settings until the exact production process has been validated on dedicated test hardware

## 2. Signing-key layout

Use two provisioned boot keys.

- BOOTKEY0: primary production release signing key
- BOOTKEY1: offline recovery signing key
- BOOTKEY2: unused, mark invalid
- BOOTKEY3: unused, mark invalid

BOOT_FLAGS1.KEY_VALID:

- KEY0 = valid
- KEY1 = valid
- KEY2 = not valid
- KEY3 = not valid

BOOT_FLAGS1.KEY_INVALID:

- KEY0 = clear
- KEY1 = clear
- KEY2 = invalid
- KEY3 = invalid

KEY_INVALID takes precedence over KEY_VALID. Unused key slots should be invalidated so they cannot later be populated as additional trusted boot keys.

## 3. Secure boot

CRIT1.SECURE_BOOT_ENABLE:

- Production target: enabled
- Provision only after all intended boot-key fingerprints are programmed and independently verified
- This must be the final secure-boot activation step, not the first step

Production firmware must be built with:

- CRYPTOMACHINE_DEV_LOGGING=OFF
- CRYPTOMACHINE_SIGN_FIRMWARE=ON
- an external secp256k1 PEM signing key
- successful picotool signature verification before release

## 4. Debug policy

CRIT1.DEBUG_DISABLE:

- Production target: enabled
- Do not program during normal firmware development
- Program only after the final production firmware and hardware test procedure is complete

CRIT1.SECURE_DEBUG_DISABLE:

- No separate dependency planned if DEBUG_DISABLE is used, because DEBUG_DISABLE disables all debug access
- Revisit only if a future authenticated debug-key workflow is intentionally designed

## 5. Boot-interface policy

BOOT_FLAGS0.DISABLE_BOOTSEL_UART_BOOT:

- Production target: enabled
- CryptoMachine Seed has no planned UART firmware-update path

BOOT_FLAGS0.DISABLE_BOOTSEL_USB_PICOBOOT_IFC:

- Production target: enabled
- Removes the PICOBOOT vendor interface from BOOTSEL mode

BOOT_FLAGS0.DISABLE_BOOTSEL_USB_MSD_IFC:

- Current V1 recommendation: leave clear initially so signed UF2 recovery remains possible
- Residual risk: Raspberry Pi erratum E21 recommends disabling both USB PICOBOOT and USB MSD for maximum mitigation against a physical fault-injection path in BOOTSEL
- Re-evaluate before final production lock
- A maximum-lock configuration may set this bit and permanently remove USB firmware updates

Important: keeping USB MSD does not authorize unsigned firmware. Secure boot must remain the execution policy.

## 6. Architecture policy

CRIT0.ARM_DISABLE:

- Must remain clear
- CryptoMachine Seed production firmware is ARM Cortex-M33

CRIT0.RISCV_DISABLE:

- Candidate production hardening setting: enabled
- Do not program until hardware validation is complete
- Rationale: CryptoMachine Seed does not plan to ship RISC-V firmware, so disabling the unused architecture can reduce unnecessary boot flexibility

BOOT_FLAGS0.DISABLE_AUTO_SWITCH_ARCH:

- Not currently required if RISC-V is disabled
- Leave clear unless later testing shows a specific reason to program it

## 7. Glitch-detector policy

CRIT1.GLITCH_DETECTOR_ENABLE:

- Candidate production hardening setting: enabled
- Must be validated on physical hardware before permanent programming

CRIT1.GLITCH_DETECTOR_SENS:

- No permanent value selected yet
- Choose only after testing boot reliability, power behavior, and normal operating conditions
- Do not assume maximum sensitivity is automatically appropriate for production

## 8. Rollback protection

BOOT_FLAGS0.ROLLBACK_REQUIRED:

- Do not intentionally enable for V1 until a formal firmware version and rollback policy exists
- The SDK header notes that this flag may be set automatically when a binary carrying rollback metadata is first booted
- Therefore, production release tooling must not add rollback metadata accidentally

Future rollback policy must define:

- rollback version numbering
- OTP rows used for rollback state
- release promotion rules
- emergency recovery behavior
- interaction with the recovery signing key

## 9. Partition-table policy

BOOT_FLAGS0.SECURE_PARTITION_TABLE:

- Leave clear unless CryptoMachine Seed adopts an RP2350 partition table

BOOT_FLAGS0.HASHED_PARTITION_TABLE:

- Leave clear unless CryptoMachine Seed adopts an unsigned-but-hashed partition table design
- A signed partition table is preferred if partitioning is ever introduced

Current V1 design does not require these fields.

## 10. Fields that must remain clear

BOOT_FLAGS0.DISABLE_FLASH_BOOT:

- Must remain clear
- CryptoMachine Seed boots production firmware from external flash

BOOT_FLAGS0.DISABLE_XIP_ACCESS_ON_SRAM_ENTRY:

- Leave clear for V1
- The SDK header warns that this can break boot ROM APIs that access XIP, including partition-table operations
- No current CryptoMachine Seed requirement justifies this irreversible restriction

CRIT0.ARM_DISABLE:

- Must remain clear

## 11. Provisioning order

No production board should be provisioned until this sequence has been rehearsed on dedicated test hardware.

Planned irreversible order:

1. Build signed production firmware with development logging disabled.
2. Verify the generated UF2 with picotool and confirm `signature: verified`.
3. Independently record and verify the intended BOOTKEY0 fingerprint.
4. Independently record and verify the intended BOOTKEY1 recovery fingerprint.
5. Program BOOTKEY0 fingerprint only.
6. Read back and verify BOOTKEY0.
7. Program BOOTKEY1 fingerprint only.
8. Read back and verify BOOTKEY1.
9. Mark KEY0 and KEY1 valid.
10. Read back and verify KEY_VALID state.
11. Mark KEY2 and KEY3 invalid.
12. Read back and verify KEY_INVALID state.
13. Program chosen boot-interface restrictions.
14. Program chosen architecture restrictions.
15. Program chosen glitch-detector settings, if approved by hardware testing.
16. Program final debug-disable policy.
17. Boot and verify a correctly signed production image.
18. Verify that an intentionally unsigned test image is rejected.
19. Verify the approved recovery/update path.
20. Only after all prior checks pass, enable CRIT1.SECURE_BOOT_ENABLE.
21. Power-cycle and repeat signed-image and recovery tests.

Exact picotool OTP write commands are intentionally omitted from this draft.

## 12. Recovery-key procedure

Normal releases are signed with BOOTKEY0.

BOOTKEY1 is kept offline as the recovery key.

If BOOTKEY0 is lost or believed compromised:

1. Stop normal release signing.
2. Build a recovery release signed by BOOTKEY1.
3. Verify the recovery artifact and signature offline.
4. Confirm that installed production units accept the BOOTKEY1-signed recovery image.
5. Only after successful recovery verification, consider revoking BOOTKEY0 by setting its KEY_INVALID bit.
6. Once BOOTKEY0 is invalidated, it cannot become trusted again.

Do not revoke the primary key before a recovery-key-signed image has been proven to boot on the target security configuration.

## 13. Signing-key handling

Production private keys must never be stored in the source repository or build directories.

Primary key:

- stored outside the repo
- backed up securely
- used only for controlled release signing

Recovery key:

- stored offline
- physically and logically separated from the primary key
- not used for normal releases

The public-key fingerprints and provisioning records are not secret, but must be treated as release-control data because provisioning the wrong fingerprint can make a secured device unable to boot intended firmware.

## 14. Current decision summary

Planned for production:

- signed firmware
- BOOTKEY0 primary key
- BOOTKEY1 offline recovery key
- BOOTKEY2 and BOOTKEY3 invalid
- secure boot enabled only after verification
- development logging disabled
- all debug disabled
- UART BOOTSEL disabled
- USB PICOBOOT disabled
- USB MSD retained initially for signed UF2 recovery, pending final E21 risk decision
- ARM enabled
- RISC-V disable considered after hardware validation
- glitch detector considered after hardware validation
- no rollback enforcement yet
- no partition-table enforcement yet
- flash boot remains enabled

## 15. Mandatory final review before OTP programming

Before any OTP programming command is approved:

- confirm RP2350 silicon revision and relevant errata
- confirm Pico SDK and picotool versions used for production
- regenerate and inspect the OTP provisioning plan
- verify both production signing keys and their fingerprints
- verify the recovery process on test hardware
- verify unsigned firmware rejection
- verify that the chosen USB recovery policy matches the product-support plan
- verify that no field marked "leave clear" is present in the provisioning payload
- review every irreversible bit one final time
