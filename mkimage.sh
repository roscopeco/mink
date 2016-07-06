
#!/bin/sh
#
# mkimage.sh - Make a disk image with Grub2 and Mink kernel.
#
# Copyright (c)2013-2016 Ross Bamford.
#
# This is run during the build process to create a bootable raw disk image 
# containing a VFAT partition with the Grub2 boot loader, the Mink kernel, 
# and everything it needs for a boot. The image can be used with Bochs, 
# Qemu, etc.
#
# This script expects that the image-staging make target has already been
# executed. It's probably best to leave it's invocation to make rather 
# than running it yourself...
#
# If you do decide to run it yourself, on most systems it'll need to be
# run as root...
#
# WARNING: This will blindly overwrite an existing image!
#
# Changes:
#	July 06 2016 - Stop hardcoding loopback device/mapping,
#	               determine during run instead.
#	               Also fix grub params to fix boot issue.
#	               (Hopefully fix #2)
#                     

# Create the actual disk image - 20MB
dd if=/dev/zero of=mink.img count=20 bs=1048576

# Make the partition table, partition and set it bootable.
parted --script mink.img mklabel msdos mkpart p ext2 1 20 set 1 boot on

# Map the partitions from the image file and find where loopback will be
looppart=`kpartx -l mink.img | awk -e '{ print $1; exit }'`
loopdev=`kpartx -l mink.img | awk -e '{ print $5; exit }'`
kpartx -a mink.img

# sleep a sec, wait for kpartx to create the device nodes
sleep 1

# Make an ext2 filesystem on the first partition.
mkfs.ext2 /dev/mapper/$looppart

# Make the mount-point
mkdir -p build/tmp/p1

# Mount the filesystem via loopback
mount /dev/mapper/$looppart build/tmp/p1

# Copy in the files from the staging directory
cp -r build/img/* build/tmp/p1

# Create a device map for grub
echo "(hd0) $loopdev" > build/tmp/device.map

# Use grub2-install to actually install Grub. The options are:
#   * No floppy polling.
#   * Use the device map we generated in the previous step.
#   * Include the basic set of modules we need in the Grub image.
#   * Install grub into the filesystem at our loopback mountpoint.
#   * Install the MBR to the loopback device itself.
grub2-install --no-floppy                                                      \
              --grub-mkdevicemap=build/tmp/device.map                          \
              --modules="biosdisk part_msdos ext2 configfile normal multiboot" \
              --root-directory=build/tmp/p1                                    \
              --boot-directory=build/tmp/p1/boot                               \
              $loopdev

# Unmount the loopback
umount build/tmp/p1

# Umap the image
kpartx -d mink.img

# hack to make everything owned by the original user, since it will currently be 
# owned by root...
LOGNAME=`who am i | awk '{print $1}'`
LOGGROUP=`groups $LOGNAME | awk '{print $3}'`
chown $LOGNAME:$LOGGROUP -R build mink.img

