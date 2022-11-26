//========= Copyright N11 Software, All rights reserved. ============//
//
// File: main.cpp
// Purpose: Main point of the kernel.
// Maintainer: aristonl, FiReLScar
//
//===================================================================//

#include <Config.hpp>
#include <IO.hpp>
#include <GDT.hpp>
#include <COM.hpp>
#include <Interrupts.hpp>
#include <Interrupts/Syscall.hpp>
#include <Interrupts/PageFault.hpp>
#include <Interrupts/DoublePageFault.hpp>
#include <BOB.hpp>
#include <CPUID.hpp>
#include <Memory/Mem_.hpp>

extern unsigned long long InfernoStart;
extern unsigned long long InfernoEnd;

__attribute__((sysv_abi)) void Inferno(BOB* bob) {
	// Create GDT
	#if EnableGDT == true
		GDT::Table GDT = {
			{0, 0, 0, 0x00, 0x00, 0},
			{0, 0, 0, 0x9a, 0xa0, 0},
			{0, 0, 0, 0x92, 0xa0, 0},
			{0, 0, 0, 0xfa, 0xa0, 0},
			{0, 0, 0, 0xf2, 0xa0, 0},
		};
		GDT::Descriptor descriptor;
		descriptor.size = sizeof(GDT)-1;
		descriptor.offset = (unsigned long long)&GDT;
		LoadGDT(&descriptor);
		kprintf("\r\e[92m[INFO] Loaded GDT...\e[0m\n\r");
	#endif

	// Create IDT
	Interrupts::CreateIDT();

	// Create a test ISR
	Interrupts::CreateISR(0x80, (void*)SyscallHandler);
	Interrupts::CreateISR(0x0E, (void*)PageFault);
	Interrupts::CreateISR(0x08, (void*)DoublePageFault);

	// Load IDT
	Interrupts::LoadIDT();
	kprintf("\r\e[92m[INFO] Loaded IDT...\e[0m\n\r");

	kprintf("Result: %x\n\r", 0x1000);

	// asm("int $0x80");
	unsigned long res = 0;
	asm volatile("int $0x80": "=a"(res): "a"(15), "d"((unsigned long)"google"): "rcx", "r11", "memory");
}

__attribute__((ms_abi)) [[noreturn]] void main(BOB* bob) {
	Inferno(bob);

	// Once finished say hello and halt
	kprintf("\e[92m[INFO] Done!\e[0m\n\r");

	while(true) asm("hlt");
}