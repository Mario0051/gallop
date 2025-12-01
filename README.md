<div align="center">

![logo](assets/logo.png)

> A hachimi module

**Trainers' Gallop U** (or libgallop) is a [Hachimi](https://hachimi.noccu.art/) module that adds some features similar to the original [Trainers' Legend G](https://github.com/MinamiChiwa/Trainers-Legend-G) to enhance the ウマ娘プリティーダービー (Umamusume: Pretty Derby) experience.

</div>

## Notice

- As this is a module, there are chances that **you might get banned by Cygames for using this**. Use at your own risk.
- For translations, look into [Hachimi](https://hachimi.noccu.art/), which by itself handles translations. This module only provides more niche features out of scope of the Hachimi project.

## Building

This uses CMake and MinGW for building the dll file (`libgallop.dll`/`gallop.dll`).

1. Get [CMake](https://cmake.org/) and [MinGW-w64](https://www.mingw-w64.org/). Make sure you are in the MinGW environment when compiling.
2. `git clone`
3. Create the build files using `cmake --preset Build -B build`.
4. Compile using `cmake --build build`.
5. A dll file should be available in the build folder. Proceed to [Usage](#usage).

## Usage

This module is meant to be used with [Hachimi(-Edge)](https://hachimi.noccu.art/).

This module works on Windows 10 and Linux (via Proton). Linux users will need to use a patched out executable and Proton versions compatible with Umamusume (GE-Proton 10-12 or Proton-CachyOS).

As of current reports, this may or may not work with DMM. Some Hachimi versions will crash with this module enabled due to GUI conflicts, so make sure to disable the GUI if you're having startup crashes!

1. Install Hachimi, setting up DLL redirection if needed.
2. In the `hachimi` folder (appears once the game is launched with Hachimi atleast once), edit `config.json` and add the path to the CarrotJuicer dll to `load_libraries`.
```json
"load_libraries": [
  "hachimi\\libgallop.dll"
],
```
3. Start the game as usual (i.e., with DMM launcher/Steam).
4. You should see a separate window when launching the game. If you see it, congratulations!

## Config

Configuration is saved in `hachimi/gallop_config.toml`.

```toml
# Enables discord RPC (if applicable)
discordRPC = true

# Lists characters to replace visually, by key
[replaceCharacters]
# For example, this would replace character ID 1067
[replaceCharacters.1067]
charaId = 1100 # Replaces them with character ID 1100
clothId = 5 # Forcibly changes all clothes used by this character to dress ID 5
replaceMini = true # Also replaces mini model if available
homeScreenOnly = false # Determines if this character is only replaced in the home screen
```

## License

Code is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).

Other miscellaneous assets are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).
