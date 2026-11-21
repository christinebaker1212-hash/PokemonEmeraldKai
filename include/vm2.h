#ifndef GUARD_VM2_H
#define GUARD_VM2_H

// Moemon Kai: the VM2 (a VMU replacement with USB, a CH340 serial port) for the VMU pet's MOeMON and Eggs
// (docs/VMU_PET_DECISIONS.md). The protocol: docs/VM2_USB_PROTOCOL.md (as the VMU manager, vmu_pet/manager/vm2link.cpp,
// which this follows). The VM2 answers only in GUI-mode (its Settings > System > GUI-mode). Blocking calls: run
// them off the game thread (src/mp/mp_vm2.c does).

#include <stddef.h>
#include <stdint.h>

#define VM2_IMAGE_SIZE 131072
#define VM2_BLOCK_SIZE 512

// Vm2_FindPort: the first CH340 serial port's name (COM5, /dev/ttyUSB0) -> 1, or 0 when there's none (quick: no
// port is opened)
int Vm2_FindPort(char *name, size_t size);
// Vm2_Open: the port, at 460800 baud or else 921600, answering @HAND00 -> 1 (then Vm2_Info has its words)
int Vm2_Open(const char *port);
void Vm2_Close(void);
int Vm2_IsOpen(void);
const char *Vm2_Info(void);              // "<HW> <FW> <ID>", or ""
// Vm2_ReadInternal: its internal 128 KB (the VMU it is), block by block
int Vm2_ReadInternal(uint8_t *image);
// Vm2_WriteBlock: one 512-byte block of its internal memory (retried until it says ACK, 5 tries)
int Vm2_WriteBlock(int block, const uint8_t *data);
// Vm2_Restart: @REST00, the VM2 starts again out of GUI-mode (the software disconnect); the port is closed
void Vm2_Restart(void);

#endif // GUARD_VM2_H
