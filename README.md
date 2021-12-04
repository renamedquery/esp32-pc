# esp32-pc

### This repo is currently a work in progress. I am currently working on the ESP32's command line.

https://user-images.githubusercontent.com/38505228/144671347-8b8e3e65-b26e-4b90-b4b4-f55994bb42a4.mp4

*Yes, it does run Bad Apple! (Make sure to run `net stop` before using `readimgseq`). A some recent updates have sped up the playback speed monumentally, and the device is now capable of ~16fps playback!*

![](https://user-images.githubusercontent.com/38505228/144322399-b0548359-7660-4e32-ba5e-7d7e87ef826c.jpg)

*Currently, the master device will now allow up to 8 slave devices to sync to it over WiFi. It can also read from an SD card filesystem.*

![](https://user-images.githubusercontent.com/38505228/144529263-3d79517f-b579-4a97-bd00-8b308f0ea439.png)

*One of the latest updates included the ability to take screenshots. The screenshot algorithm will be improved later to use run length encoding, which should speed it up significantly.*

![](https://user-images.githubusercontent.com/38505228/144286600-0376f26c-2630-45b0-bb09-f7fb35e8bbb3.jpg)

*I call it, "The Signal Integrity House of Horrors".*

### Specs:

- 520KB RAM
- 448KB ROM
- 240MHz processor clock speed
- 3 bit VGA colors (RGB)
- 640x350 output resolution
- 6 physical ESP32 devices

### Parts:

- 6x ESP32-S2 Dev Kit
- 1x Digital PmodSD

### Commands:

- fbmem
- fbinfo
- hwinfo
- lsdev
- net <command_args>
- df
- sd <command_args>
- mkdir <absolute_path>
- ls
- touch <absolute_path>
- cd <absolute_path>
- readimgseq <absolute_path>
- cls
- screenshot <absolute_path>

*Use the `help` command to list all of the commands and their usage. You can comment out text by writing `nop <string>`.*

### Why am I doing this?

I have quite a bit of experience programming on high end desktop computers, and thought that it would be useful to learn how to program on a system with much tighter tolerances and lower specs.
