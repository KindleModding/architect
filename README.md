# Architect
Architect is a utility that wraps one or more binaries allowing the resulting binary to be run on both ARMHF and ARMEL Kindles.

## Usage
1. Compile this program using `meson setup builddir_ARCH` & `meson compile -C builddir_ARCH`
2. Add the armel and armhf target binaries to the compiled program:
   1. objcopy --add-section .sf_payload=target_sf architect architect
   2. objcopy --add-section .hf_payload=target_hf architect architect
3. Done! The final binary will now run on both `armel` and `armhf` Kindles

## Notes
The `hf_payload` section is optional as `armel` binaries should be able to run under the `armel` interpreter.

### How does it work
1. Check system architecture
2. Get the required interpreter and binary data
3. Create a temporary file & write target payload to it
4. Fork
   1. Execute the temporary file
5. Wait for subprocess to exit
6. Delete the temporary file