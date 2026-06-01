# Linux Flexxon tool

This tool allows you to use Flexxon [X-mask](https://www.flexxon.com/x-mask/)
on Linux systems.

## Warning

This doesn't require the password. It uses a vulnerability on these cards that
allow you to disable the masking protection.

Also, once unlocked, the protection is fully disabled, even if you remove the
card. You need to lock it once done to restore the "protection".

# Building

```
make
```

# Running

## Unlocking a card

```
./flexxon-tool --device /dev/mmcblk0 --unlock
```

## Locking a card

```
./flexxon-tool --device /dev/mmcblk0 --lock
```

# AI disclaimer

Part of this code was AI-generated using Copilot. To be more specific, I used
AI to convert the encoding algorithm from my Python script to C.
