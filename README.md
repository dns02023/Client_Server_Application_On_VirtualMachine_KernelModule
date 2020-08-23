# Client_Server_Application_On_VirtualMachine_KernelModule

# Simple Client-Server Application On VirtualBox

3 Virtual Machines (2 client VMs, 1 server VM) / Kernel Moudle

Each client VMs can register their IP address and hostname to server VM (also can remove too).

Each client VMs can ask to server VM about the peer IP address using peer’s host name and save received data as cache


Each VMs communicate each other with UDP socket.

Server VM's user application and Kernel communicate each other with Netlink socket


This project shows basic network look-up mechanism


In client VMs, run dns_client.c.

In server VM, run dns_netlink_server.c.







