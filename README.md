# RayCompiler 🚀

[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Framework](https://img.shields.io/badge/Framework-LLVM-yellow.svg)](https://llvm.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

一個使用 C++ 和 LLVM 框架從頭打造的簡易 Just-In-Time (JIT) 編譯器。本專案旨在實現一個具備函式定義、條件表達式和二元運算子的基本語言。

---

## 專案介紹

RayCompiler 遵循經典的編譯器架構，將原始碼逐步轉換為可執行的機器碼。

1.  **詞法分析 (Lexer)**: `lexer.cpp` 將原始碼字元流分解為一系列的 token（如：數字、關鍵字、運算子）。
2.  **語法分析 (Parser)**: `parser.cpp` 使用詞法分析器產生的 token 來建構一個抽象語法樹 (AST)。AST 是程式碼結構的樹狀表示。
3.  **程式碼產生 (Codegen)**: `codegen.cpp` 遍歷 AST，並使用 LLVM IR Builder 產生 LLVM 中介碼 (Intermediate Representation)。最後，LLVM 的 JIT 引擎會將此 IR 編譯成原生機器碼並執行。

這個專案是學習編譯器原理、LLVM 框架以及現代 C++ 實踐的一個絕佳範例。

---

## ✨ 功能特性

- **JIT 即時編譯**: 使用 LLVM MCJIT 引擎將程式碼直接編譯並在記憶體中執行。
- **函式系統**: 支援使用者自訂函式，包含參數與回傳值。
- **表達式解析**:
  - 支援 `+`, `-`, `*`, `/` 等二元運算子，並處理其優先級。
  - 支援 `<` 和 `>` 比較運算子。
  - 支援變數賦值 `=`。
- **流程控制**: 實現了 `if/then/else` 條件表達式。
- **模組化設計**: 清晰劃分詞法、語法和程式碼產生器，易於擴充。

---

## 🚀 快速開始

在開始之前，請確保您的開發環境已安裝必要的工具。

### 先決條件

- C++ 編譯器 (建議使用 `clang++`，與 LLVM 相容性最佳)
- LLVM 開發函式庫 (請確保 `llvm-config` 工具在您的 PATH 中)

在基於 Debian/Ubuntu 的系統上，您可以透過以下指令安裝：
```bash
sudo apt-get update
sudo apt-get install clang llvm-dev
````

### 🛠️ 編譯

1.  Clone 此專案：

    ```bash
    git clone git@github.com:rayhuang2006/RayCompiler.git
    cd RayCompiler
    ```

2.  使用 `clang++` 和 `llvm-config` 進行編譯。`llvm-config` 會自動提供所需的標頭檔路徑和連結器旗標：

    ```bash
    clang++ -std=c++17 src/*.cpp -Iinclude `llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native` -o ray_compiler
    ```

### 💻 使用方法

編譯器會從標準輸入讀取原始碼。您可以將程式碼儲存在一個檔案中 (例如 `test.ray`)，然後使用管線 (pipe) 導入。

1.  建立一個範例檔案 `test.ray`：

    ```cpp
    # test.ray - 一個簡單的範例

    # 計算斐波那契數列
    def fib(x)
        if x < 3 then
            1
        else
            fib(x-1) + fib(x-2);

    # 呼叫函式並印出結果
    fib(10);
    ```

2.  執行編譯器：

    ```bash
    ./ray_compiler < test.ray
    ```

3.  編譯器將會即時編譯並執行程式碼，輸出 LLVM IR 以及執行結果。

-----

## 📂 專案結構

```
RayCompiler/
├── .git/
├── include/
│   ├── ast.h         # 抽象語法樹節點定義
│   ├── lexer.h       # 詞法分析器介面
│   └── parser.h      # 語法分析器介面
├── src/
│   ├── codegen.cpp   # 程式碼產生器實作
│   ├── lexer.cpp     # 詞法分析器實作
│   └── parser.cpp    # 語法分析器實作
├── .gitignore
└── README.md
```

-----

## 📜 授權條款

本專案採用 [MIT License](https://opensource.org/licenses/MIT) 授權。