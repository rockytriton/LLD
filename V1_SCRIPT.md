Welcome to the Low Level Devel channel, this is going to be my first set of videos which will be related to low level development on the raspberry PI hardware.

This first set of videos is going to be based on a set of tutorials by Serge Matyukevich on github, I'll paste the link in the description:
https://github.com/s-matyukevich/raspberry-pi-os

It's a very interesting set of tutorials which goes in depth on how things are implemented in linux as well.  In these videos I will focus only on the raspberry PI side of things but will also show
you where to find the information in the datasheets and device tree files that you need to do this sort of bare metal development.

I'm going to try to keep these videos fairly short, maybe 20 to 30 minutes so I will separate them into multile sections.
In this first section, we are going to setup the main project and just start with some of the boot process code.  
I'm going to assume that you have at least some experience with C and assembly programming, so I'm not going to spend time explaining what all of the code does unless it's directly related
to the hardware.  I'm going to be using visual studio code for the development and linux for the operating system.  If you are running windows, I would suggest you create a
linux virtual machine.  If you don't want to create a linux virtual machine and are running on windows, there are other ways to run the cross compiler, for instance following Serge's method of
using docker, I will leave that process up to you.

As a first step, we need to install a cross-compiler to compile ARM code on an intel machine.

If you are running a debian based OS, you can run the following command:

sudo apt install gcc-aarch64-linux-gnu

Any other non-debian based OS, I assume you know how to find a cross compiler for your system.

Now let's create the basic structure of the project.  I'll create a new directory for the project and change into that directory.

then create a couple of directories, src and include.
We'll create a Makefile, along with some of the initial headers and source files, just to get things started here.

I like to create a common header to be used in all files for some common types as well.  The code developed in here will be very
similar to Serge's lessons, but I will diverge from some of his design a little bit.

So now let's start with the makefile, this is going to be how we build the project.

I'm going to start with adding a variable for the rapsberry pi version, because I want to 
be able to build for both the RPI 3 and 4.

Next I'm going to create a variable for the boot mount, where I will copy the build files
after compiling.

Now we setup the base name for the cross compiler which we installed.  Yours may be 
slightly different if you are doing your cross compilation different than me.

Next we create the options to pass to the C compiler.  Since this is bare metal, we
will not have the standard libraries and it will be free standing.  We want to pass 
our include directory as well and specify to use general registers only.

For the assembly options we also need to specify our include path.

We'll setup build and source directories.

now we start with the build sections.  For all we will build the kernel image. 
For clean we will cleanup the build directory and image file.

Now for all c compiled objects, we will reference their C files.  be sure to 
create their build directory if it doesn't exist.
Then we call the arm cross compiler gcc with the c options, specify to compile only
and the output file.

As we do the same for S, assembly files.

Now we want to create some variables for the files we are going to be compiling and
building.
Using the wildcard for all C files, and the same for asm files.

Now we will create out object files from the C files and asm files.

And a section for our dependency files.

Now for the section on our kernel image.  It's going to depend 
on the linker script and object files.
We'll add some echo output to display the variables.
Now we run the cross compiler's ld command to link the object files
into one executable elf file.  Then we want to convert that elf
file to a raw binary executable image.

Now if we are building the pi 4 image, we want to rename the 
kernel file so we can have both on disk at the same time.
otherwise we keep the default kernel8.img filename.
And these files get copied to our mounted boot partition on the
sd card, which we will cover in more details later.
Then finally copy the config.txt file, which we will also cover
later and run the sync command to ensure all writing to the disk
is complete.

Ok so now lets test out our makefile, first we open up a new terminal window.
I run make clean first just in case any previous build files were there.  Then just run make.
Now you can see from the output of make that the files we created were compiled, even though they are empty and it runs through all of
the build steps and it looks like there was one error here due to a missing config.txt file, so lets go ahead and create the config.txt

First we need to specify that we are using arm 64 bit so that it picks up the correct kernel file.
next I like to add in some uart 2nd stage debug info that it supplied by the firmware, that helps to debug if any uart issues are
code related or just the hardware not working.
Then I add this miniart-bt overlay because without it sometimes the bluetooth device will conflict with the mini uart.

Then a section specific to pi4 where we specify an alternate name for the kernel image.

Now if we rerun the make command, we can see that it succeeds.

Now lets start writing some of the code, we'll start with the base.h under peripherals.

First I put pragma once at the top, which is a way to tell the compiler to only include the file ones, much shorter than using ifdefs
and very widely supported.

Now we will create a section specific to the PI 3 board.
And will fill in the peripheral base address

And a section for PI 4, with its base.

Then finally an else section that will break the build if the version is not supported.

...

Now, I'll show you where these values come from.  First we go to the raspberry pi documentation site.
I'll put the url in the description.

Under hardware, raspberry pi, the pi 3 is a bcm 2837, which is a modified 2835.  The 2835 has a full data sheet here, which you should download
for reference.
And for the PI 4, its a BCM2711, so download that ones full datasheet as well.

Now, opening the datasheet for the 2835, we go down to the section 1.2.3 where it show s the physical addresses.
This section says the address range starting at 0x20000000 is for peripherals, and the 7E00 0000
is the bus address, so when peripherals show a bus address starting with 7E, we translate that to 20.

Now for the PI4, it also defines this data, but instead of using this, we can actually use the 
device tree information from the raspberry pi linux source tree.

I'll paste this in the comments section.

So we go into the linux, arch, arm, boot, dts section.

here we will find the dts files for the rasperry pi.

Remember the bcm2835 is what is used as a base, so we can open this dtsi file
and in here we see the soc section, which has a section for ranges.

In the ranges we can see the bus address and the base peripheral address.
But wait, we put in 3F as the start of the address, that's because the PI3 is a 2837, 
which is a modified 2835 and has its own dtsi file.

So if we open this dtsi file, go to the soc ranges, here we see 3F as the start of the address for
peripherals.

Now we can go to the 2711 soc dtsi file and we will find here we have the same range which has the 
bus address, followed by the peripheral address starting with FE.

So that's where these two important addresses are actually coming from.

Ok, that's enough for this first video, in the next video we will continue on with more of the header files.
