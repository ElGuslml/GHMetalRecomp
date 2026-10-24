# Game files

Put the files of your own copy of **Guitar Hero: Metallica (Xbox 360)** here, so that this file exists:

```
assets/default.xex
```

The whole disc content goes in this folder. Extract it from your disc or ISO with
[extract-xiso](https://github.com/XboxDev/extract-xiso/releases).

The build needs `default.xex` (the code is recompiled from it); the game needs everything else.
The expected executable has CRC32 `A7C93B87` (10,366,976 bytes, title ID `41560830`); `GHMetallicaRecomp.exe`
warns at startup if yours is different.

Nothing in this folder is tracked by git.
