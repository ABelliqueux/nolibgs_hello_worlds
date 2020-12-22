# Nolibgs Hello Worlds !

So you want to begin developping on the original PSX but don't know where to start ?

This repo is destined to host a bunch of simple examples, each describing how to do one thing.

The code here will be using PsyQ, the "Official" Sony SDK, but we will not be using libGS, the Extended Graphics Library.

Instead we'll try to devises methods to reproduce libgs functions. This will not necessarly more efficient, but we'll learn
a lot more stuff !

 
## Setting up the SDK : Modern GCC + PsyQ

For this we'll rely heavily on grumpy-coder' s pcsx-redux, which will provide us with:

  * A way to compile the code with a modern version of GCC
  * An emulator with a lot of debugging features
  
### Let's do it !

  * On windows, install WSL2 and Debian then launch a GNU/Linux terminal

  1. Install the needed software packages ( aka dependencies in Linux world ) :

```bash
sudo apt-get install -y git make pkg-config clang-10 g++-9 gcc-mipsel-linux-gnu g++-mipsel-linux-gnu binutils-mipsel-linux-gnu libavcodec-dev libavformat-dev libavutil-dev libglfw3-dev libsdl2-dev libswresample-dev libuv1-dev zlib1g-dev
```
  
  2. Clone the pcsx-redux repo : 
  
```bash
git clone https://github.com/grumpycoders/pcsx-redux.git --recursive
```
  
  3. Compile pcsx-redux : 
  
```bash 
cd pcsx-redux && ./configure && make
```
  
  4. Get the converted PsyQ 4.7 libs : 
  
```bash
wget http://psx.arthus.net/sdk/Psy-Q/psyq-4.7-converted-full.7z
```
  
  5. Extract to [...]pcsx-redux/src/mips/psyq/ : 

```bash
7z x -o/pcsx-redux/src/mips/psyq/
```
  
  6. That's it ! After that you can check everything's working byt trying to compile some example code :
  
```bash 
cd pcsx-redux/src/mips/psyq/
make 
```

You should know have a 'cube.ps-exe' file in the folder. This a PSX executable you can load with most emulators.


## Embedding binary data in a ps-exe

So, if you don't know it yet, the fun in PSX development is to be able to upload your exes on real hardware with a USB/Serial cable.
This means that the data you'll use in your program ( graphics, sounds, etc.) will have to be embedded in your exe in a binary form, 
as you won't be able to stream them from the serial port. 

*Well technically you could load them in memory before uploading your exe or stream them from a cd, but let's keep things simple for now.*

With our setup, this is quite easy !

  1. In `pcsx-redux/src/mips/common.mk` , add the lines :
  
  ```mk
# convert TIM file to bin
%.o: %.tim
	$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O elf32-tradlittlemips -B mips $< $@

# convert VAG files to bin
%.o: %.vag
	$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O elf32-tradlittlemips -B mips $< $@
```

If you pay attention, you can see that's the same command, but for different file types. TIM files are bitmap images and VAG is the sound format used in this example.
Each time you'll want to add a file type, just duplicate and change `%.vag` to `%.filetype`

Then, in your project folder, copy the makefile from the cube example :

```bash
mkdir new_project && cd new_project
cp ../cube/Makefile ../
```

All you have to do now is add the files you wish to embed to the SRCS variable, without forgetting the \ :

```bash
SRCS = main.c \
../common/crt0/crt0.s \
file_to_embed.ext \
```

  2. So this part takes care of converting our data to binary. Now to access them from your program, just use this in your sources :
```c
extern ulong _binary_filename_extension_start[]; 
extern ulong _binary_filename_extension_end[];
extern ulong _binary_bowsht_tim_length[];
```

The filename variable must begin with `_binary_` followed by the full path of your file, with . and / replaced by _ (underscore), and end with `_start[];` or `_end[];` or `_length[];` [source](https://discord.com/channels/642647820683444236/663664210525290507/780866265077383189)

`_start` and `_end` are pointers, while `_length` is a constant.

That's it! When you'll type `make` next time, it should convert your files to .o, then include them in your ps-exe.

# Links and Doc

  * [https://psx.arthus.net/starting.html](Getting started)
  * [http://psx.arthus.net/sdk/Psy-Q/DOCS/](PsyQ Doc)
  * [https://ps1.consoledev.net/](Ps1 dev links)
  * [http://psxdev.net/](psxdev.net)
  * [https://discord.com/invite/N2mmwp?utm_source=Discord%20Widget&utm_medium=Connect](psxdev Discord)
