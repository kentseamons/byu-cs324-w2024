# Install Linux on a VM

The point of this exercise is to install a Linux-based OS on a Virtual Machine
(VM) for development in this class.

Notes:
 - While you can develop, compile, and test on this VM, you must eventually
   compile and test your code on the BYU CS lab machines before submitting it.
   They are quite possibly a different kernel and architecture!
 - If using UTM/Qemu on MacOS (Apple M1 or M2 chip) solution, some of the
   binaries provided will not run on your VM system, as they were compiled to
   be run on the BYU CS lab machines.


# VirtualBox (amd64 only)

 1. Download and install
    [VirtualBox](https://www.virtualbox.org/wiki/Downloads).

 2. Download the "netinst" (net install) image with amd64 architecture from
    [Debian](https://www.debian.org/releases/stable/debian-installer/).

 3. Start VirtualBox, and click "New" to create a new VM.  Give the machine 2GB
    (2048 MB) of RAM and a dynamically-allocated hard drive with at least 20GB
    of disk space.  Using the default for the other options should be fine.
    Start the VM, and select the install image (`.iso` file) you downloaded
    when prompted for a startup disk.

 4. Go through the installation using all the default options (you will have to
    explicitly select "yes" to write changes to disk), until you come to the
    "Software Selection" menu.  At that menu, un-check the "GNOME" box, and
    check the "LXDE" box. LXDE provides a lightweight desktop environment that
    demands less of your host system.  You will need to explicitly tell the
    installer to install GRUB to the hard drive.

 5. Reboot the VM when prompted, then log in.

 6. Open a terminal (`LXTerminal`) and run the following from the command line
    to temporarily become `root` (system administrator):

    ```
    su -
    ```

    From the `root` (`#`) prompt, add your user to the `sudo` group:

    ```
    # usermod -a -G sudo $USER
    ```

    Now log out of LXDE and log back in.  As a member of the `sudo` group, you
    will be able to run commands that require administrator privileges on a
    command-by-command basis using `sudo`, rather than working as the `root`
    user, which is discouraged.

 7. On the host machine, select "Devices" from the VirtualBox menu, then select
    "Insert Guest Additions CD Image..."

 8. Within the guest OS, open a terminal, and run the following from the command
    line to mount the CD volume:

    ```
    mount /media/cdrom
    ```

    Then run the following commands to build and install the VirtualBox Guest
    Additions for your VM:

    ```
    sudo apt install linux-headers-amd64 build-essential
    sudo sh /media/cdrom/VBoxLinuxAdditions.run
    ```

    This will allow you to do things like set up a shared drive between host and
    guest OS and use a shared clipboard.

 9. Reboot your VM to have the changes take effect.

 10. On the host machine, select "Devices" from the VirtualBox menu, then
     select "Shared Folders", then "Shared Folders Settings...".  Click the
     button to add a shared folder, then choose which host folder to share
     (e.g., `/Users/$USER/VMshared`, where your actual username replaces
     `$USER`) and where it will mount on the guest filesystem (e.g.,
     `/home/$USER/host`).  Selecting both "Auto-mount" and "Make permanent" is
     recommended.  For more information see the
     [official documentation](https://docs.oracle.com/en/virtualization/virtualbox/7.0/user/guestadditions.html#sharedfolders).
 
 11. From the prompt, add your user to the `vboxsf` (VirtualBox shared folders)
     group:

     ```
     sudo usermod -a -G vboxsf $USER
     ```

     Now log out of LXDE and log back in.  As a member of the `vboxsf` group,
     you will be able to access the folder `/Users/$USER/VMshared` (or
     whichever folder you selected) on the host from `/home/$USER/host` (or
     whichever mount point you selected) in the VM.

 12. On the host machine, select "Devices" from the VirtualBox menu, then
     select "Shared Clipboard", then "Bidirectional". This will allow you to
     "copy" items from the host machine and "paste" them into the VM or
     vice-versa.

 13. Run the following to remove some unnecessary packages from your VM:

     ```
     sudo apt purge libreoffice-{impress,math,writer,draw,base-core,core,help-common,core-nogui} xscreensaver
     sudo apt autoremove
     ```

 14. Disable the screen locker by doing the following:
     - Select "Preferences" then "Desktop Session Settings" from the menu.
     - Uncheck the box titled "Screen Locker," and click "OK".
     - Log out of LXDE and log back in.

 15. Run the following to install a few packages that will be useful for you in
     this class:

     ```
     sudo apt install git tmux vim
     ```

 16. Install whatever other tools and utilities that you think will improve your
     development environment.  Please note that if you have configured shared
     folders as described above, you can use whatever development environment you
     have already installed on your host to manipulate files in
     `/home/$USER/host` or some subfolder thereof.  Thus, you do not have to
     develop within the VM itself if you do not want to.


# UTM/Qemu on MacOS

 1. Install [Homebrew](https://brew.sh/).

 2. Install qemu and utm:
    ```bash
    brew install utm qemu
    ```

 3. Download the "netinst" (net install) image from
    [Debian](https://www.debian.org/releases/stable/debian-installer/).
    For M1/M2 hardware, use the arm64 architecture.  For anything else, use
    amd64.

 4. Start UTM, then do the following:

    a. Click the "+" button to create a new virtual machine.

    b. Start: Click "Virtualize"

    c. Operating System: Click "Linux".

    d. Linux: Under "Boot ISO Image", click "Browse", then select the install
       image (`.iso` file) you downloaded.  Then click "Continue".

    e. Hardware: Specify at least 2048 MB RAM, then click "Continue".

    f. Storage: Specify at least 20GB, then click "Continue".

    g. Shared Directory: Select a directory that will be shared between the
       guest OS and your VM (e.g., `/Users/$USER/VMshared`, where your actual
       username replaces `$USER`).  Then click "Continue".

    h. Click "Play".

 5. Read the note immediately below, then follow steps 4 through 6 from the
    [VirtualBox instructions](#virtualbox-amd64-only).

    Note: Before rebooting (step 5), do the following to "remove" the install CD:

    a. Click the "Drive Image Options" button.

    b. Select "CD/DVD (ISO) Image".

    c. Click "Eject".

 6. Within the guest OS, open a terminal, and run the following from the command
    to install utilities for allowing the host to interact with the guest:

    ```bash
    sudo apt install spice-vdagent
    ```

 7. Reboot your VM to have the changes take effect.

 8. Mount the shared directory.

    a. First create a mount point on the VM:

       ```bash
       sudo mkdir /media/shared
       ```

    b. Mount the shared volume as type `9p`:

       ```bash
       sudo mount -t 9p -o trans=virtio,version=9p2000.L share /media/shared/
       ```

    c. Change the permissions (from only the VM perspective) on files and
       directors in the shared directory, so your user (in the guest system) can
       access the files.

       ```bash
       sudo chown -R $USER /media/shared/
       ```

    d. Test your new mount by listing directory contents:

       ```bash
       ls -l /media/shared
       ```

    e. Add the following line to `/etc/fstab` such that the shared volume is
       mounted automatically at boot:

       ```bash
       share	/media/shared	9p	trans=virtio,version=9p2000.L,rw,_netdev,nofail	0	0
       ```

    f. Optionally create a symbolic link to the share mount from your home
       directory:

       ```bash
       ln -s /media/shared/ ~/shared
       ls -l ~/shared
       ```


 9. Follow steps 13 through 16 from the
    [VirtualBox instructions](#virtualbox-amd64-only).
