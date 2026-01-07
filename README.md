# Terminal AI Client (C + Gemini)

A lightweight, terminal-based chat client written in C.
This tool connects to the **Google Gemini API** to act as an AI assistant directly from your command line.

> **Status:** Active Development

## 🚀 Features
- **Direct API Integration:** Connects to Google's Gemini models using `libcurl`.
- **JSON Parsing:** Robust parsing of complex API responses using `jansson`.
- **Secure:** Loads API credentials from environment variables (no hardcoded keys).
- **Memory Efficient:** Manual memory management for optimal performance.

## 🛠️ Tech Stack
- **Language:** C (C99 Standard)
- **Libraries:**
  - `libcurl`: Network requests.
  - `jansson`: JSON parsing.

## 📦 Installation & Usage

### 1. Prerequisites
You need `libcurl` and `jansson` installed.
An API key 
**macOS (Homebrew):**
```
bash
brew install curl jansson
export GEMINI_API_KEY="your_api_key_here"
gcc main.c -o [file_name] -lcurl -ljansson
```

## ⚙️ How It Works
1.  **Input:** Reads user query from `stdin`.
2.  **Request:** Constructs a JSON payload (`{"contents": ...}`) matching Gemini's schema.
3.  **Network:** Sends a POST request to `generativelanguage.googleapis.com`.
4.  **Parse:** Extracts the `"text"` field from the nested JSON response.

## 📂 Project Structure
```.
├── main.c           # Core logic
├── README.md        # Documentation
└── .gitignore       # Build artifacts