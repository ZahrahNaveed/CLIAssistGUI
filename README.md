# Nova - Windows Desktop Assistant (C++)

Nova is a simple personal assistant for Windows built using C++. It provides a native GUI with basic voice input and text-to-speech functionality using Microsoft's Speech API (SAPI).

---

## Features

- Type or speak commands using your microphone
- Converts voice to text using speech recognition
- Responds with text and reads replies out loud
- Simple Windows GUI with scrollable output

---

## Technologies Used

- C++ and Win32 API
- Microsoft SAPI (Speech SDK)
- ATL COM (for smart pointers)
- Visual Studio (tested with 2022)

---

## How to Build and Run

1. Clone this repository
2. Open the `CLIAssistGUI.sln` file in Visual Studio
3. Set configuration to `Debug` and platform to `x64`
4. Build the solution and run the app

> Requires Windows with SAPI installed (usually included by default)

---

## Example Commands

You can type or speak commands like:

- `What time is it`
- `Open Notepad`
- `Tell me a joke`

You can customize responses in `ProcessCommand()` (inside `Assistant.cpp`).



