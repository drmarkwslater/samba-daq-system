#include <stdio.h>
#include <stdlib.h>
#include <hw/pci.h>

main()
{
    struct pci_dev_info info;
    void            *hdl;
    int         i;

    memset(&info, 0, sizeof (info));

    if (pci_attach(0) < 0) {
        perror("pci_attach");
        exit(EXIT_FAILURE);
    }

    /*
     * Fill in the Vendor and Device ID for a 3dfx VooDoo3
     * graphics adapter.
     */
    info.VendorId = 0x121a;
    info.DeviceId = 5;
 
    info.VendorId = 0x10b7;   /* 3Com Corporation */
    info.DeviceId = 0x9050;    /* 3C905-TX Fast Etherlink XL PCI 10/100 */
  
    info.VendorId = 0x5253;   /* CN .. */
    info.DeviceId = 0x434E;   /* .. RS */

    if ((hdl = pci_attach_device(0,
        PCI_SHARE|PCI_INIT_ALL, 0, &info)) == 0) {
        perror("pci_attach_device");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 6; i++) {
        if (info.BaseAddressSize[i] > 0)
            printf("Aperture %d: "
                "Base 0x%llx Length %d bytes Type %s\n", i,
                PCI_IS_MEM(info.CpuBaseAddress[i]) ?
                PCI_MEM_ADDR(info.CpuBaseAddress[i]) :
                PCI_IO_ADDR(info.CpuBaseAddress[i]),
                info.BaseAddressSize[i],
                PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
    }

    printf("IRQ 0x%x\n", info.Irq);

    pci_detach_device(hdl);
}
