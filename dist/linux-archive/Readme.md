Creating a Linux release archive in VM
======================================

Prerequisites
-------------

1. A computer with lots of RAM
	- If you can't spare, say 5GB of RAM, you need to install Ubuntu (this guide uses "live" version and does not cover installing the OS, but it's relatively simple)
	- If you at any point get an error about full disk space in the VM, then you have allocated too little RAM for the machine
2. VirtualBox https://www.virtualbox.org/
3. Ubuntu desktop disk image(s) for desired distro https://releases.ubuntu.com/
	- Earliest with packaged Ogre 1.8 is Ubuntu 12.10

Setup
-----

1. In VirtualBox, add a new Ubuntu VM with as much memory as you can spare. Virtual hard disk should not be needed (unless you are installing the OS).
2. Configure the VM by right clicking it and selecting Settings:
	- In System -> Processors, allocate more processors if you can spare
	- Probably a good idea to add video memory in Display -> Video
	- In Storage, add the Ubuntu .iso disk image file to the virtual CD-ROM drive
3. Fire up the VM and choose your language when it asks it. Then pick "Try Ubuntu", which will take you to the desktop

SR dependencies
---------------

1. Search for "Software Sources" app from the Dash (top left corner) and enable Universe and Multiverse repos from there
2. Download the dependencies.sh script in this folder from github or transfer it to the VM in another way.
3. Execute the dependencies.sh script

Compiling SR and creating the archive
-------------------------------------

1. cd ~/stuntrally/dist/linux-archive
2. ./make-archive.sh

Final words
-----------

Since we used a live version of Ubuntu, all the data is lost if you shut the VM down. If you want to use the environment again, either use VirtualBox's ability to save VM state or do a full-blown virtual installation of Ubuntu.

