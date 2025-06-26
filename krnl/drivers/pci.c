#include "pci.h"
#include "vga/vga.h"
#include "interrupts/pic.h"
#include "io.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_ADDRESS);
}

static uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t func){
    uint32_t r = pci_read_config(bus, device, func, 0);
    return (uint16_t)(r & 0xFFFF);
}

static uint16_t pci_get_device_id(uint8_t bus, uint8_t device, uint8_t func){
    uint32_t r = pci_read_config(bus, device, func, 0);
    return (uint16_t)(r >> 16);
}

void pci_scan(void){
    printString("Scanning PCI bus...", 0x07);
    for(uint16_t bus = 0; bus < 256; bus++){
        for(uint8_t device = 0; device < 32; device++){
            uint16_t vendor_id = pci_get_vendor_id(bus, device, 0);
            if(vendor_id == 0xFFFF){
                continue;
            }
            uint16_t device_id = pci_get_device_id(bus, device, 0);
            printString("device: bus=", 0x07);
            printDec(bus, 0x07);
            printString(", dev=", 0x07);
            printDec(device, 0x07);
            printString(", vendor=", 0x07);
            printHex(vendor_id, 0x07);
            printString(", device=", 0x07);
            printHex(device_id, 0x07);
            printString("\n", 0x07);
        }
    }
}