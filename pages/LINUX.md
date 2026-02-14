# How to compile (Linux)
Compiling this EFI application is incredibly simple. All you need is a working rolling Linux distro.

First download and install gcc, gnu-efi.
```
sudo apt install gcc gnu-efi build-essential cmake git
```
This should install all required packages that you will need to build.

Clone this repo.
```
git clone https://github.com/Tuhtarov/negativespoofer
```

Rename repo folder.
```
mv ./negativespoofer ./hwid
```

Build the application.
```
cd hwid && ./scripts/build.sh
```

Now you should see hwid.efi in the build folder. Congrats!