## About this Project

**JaPass** is a simple password manager developed as part of a university software project. 
The goal of the project is to securely store and manage passwords locally.

The project is implemented mostly in **C++**, uses the **Qt** framework for the user interface, 
**CMake** as its build system, and **SQLite** as its database. 
For security, it uses **Argon2id** for key derivation from the master password and **ChaCha20** for encrypting the password database.

### Password File Format

The password file is structured as follows:

| Field | Size (bytes) | Value                                  |
|-------|--------------|-----------------------------------------|
| 01    | 8            | `JAPASSDB` (Magic ID)                   |
| 02    | 4            | `01` (Version of file format)           |
| 03    | 16           | Argon2id Salt                           |
| 04    | 12           | Argon2id Parameters                     |
| 05    | 12           | Nonce for ChaCha20                      |
| 06    | ?            | Encrypted password database             |

### Requirements
- CMake
- A C++ compiler (e.g. GCC/Clang/MSVC)
- Qt (development libraries)
- SQLite (development libraries)

> ⚠️ This project was created as part of a university course and is primarily intended for learning/demonstration purposes.
