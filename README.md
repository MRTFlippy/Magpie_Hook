# It's [Magpie](https://github.com/Blinue/Magpie), but as a DLL Hook instead of a standalone EXE

## Why?
Because it was easier to distribute and tell people to press a hotkey to upscale instead of downloading an external application and running it everytime.

## About
This project hooks into `MagpieRT.dll` within the game application process and triggers window upscaling via a hotkey. Settings (such as scaling modes or keys) can be modified via the `.ini` file.

This project is based on **Magpie v0.8**.
*   Original Magpie v0.8 Source: [View here](https://github.com/Blinue/Magpie/tree/release/v0.8)

## How to Build & Use

### 1. Building the DLL
1.  Open **Visual Studio**.
2.  Create a new **C++ DLL Project** named `Magpie_Hook`.
3.  Copy the `dllmain.cpp` source code provided in this repository into your project.
4.  Compile the DLL (Ensure you select the correct architecture, e.g., x64 or x86).

### 2. Setup
1.  Download **Magpie v0.8** (or compile it from source).
2.  Locate the game directory you wish to hook.
3.  Copy the `effects` folder and `MagpieRT.dll` from the Magpie v0.8 release into the game's directory.
4.  Copy your compiled `Magpie_Hook.dll` and the `Magpie_Hook.ini` file to the game's directory.

### 3. Injection
You can enable the hook using one of two methods:
*   **Method A:** Use a DLL Injector to inject `Magpie_Hook.dll` into the running game process.
*   **Method B:** If you have access to the game source code, load the library manually:
    ```cpp
    LoadLibraryA("Magpie_Hook.dll");
    ```

---

## ⚠️ NOTE

**Magpie by default is distributed with a 64-bit RT DLL.**

If the application/game you are targeting is **32-bit (x86)**, the default 64-bit `MagpieRT.dll` will not work. You must:
1.  Download the [Magpie v0.8 source code](https://github.com/Blinue/Magpie/tree/release/v0.8).
2.  Compile the Runtime (RT) in **x86/32-bit** mode.
3.  Use that resulting DLL.

---

## Known Issues & Bugs

*   **Toggle Issue:** After activating the upscale with the hotkey, pressing the hotkey again **does not** revert the window to original size. Unsure why this occurs.
    *   *Workaround:* You must **ALT+TAB** out of the window to revert the effect.
