
# myHeal

## 簡介

`myHeal` 是一個命令列工具，用於將先前使用 `myBreak` 分割成多個小檔案的檔案重新組合成原始檔案。它需要指定目標檔案名稱和分割檔案時使用的前綴，並會自動讀取由 `myBreak` 產生的 meta 檔案 (`<prefix>.meta`) 來獲取重建檔案所需資訊。

## 編譯

要編譯 `myHeal` 程式，請確保你的系統上已安裝 GCC 編譯器。在包含 `myHeal.c` 和 `Makefile` 的目錄中，執行以下命令：

```bash
make
```

這將會編譯 `myHeal.c` 並產生一個名為 `myheal` 的可執行檔。

要清理編譯產生的可執行檔，可以執行：

```bash
make clean
```

## 使用方式

```bash
./myheal <destination> <prefix>
```

* `<destination>`: 重新組裝後的目标檔案名稱。
* `<prefix>`: 分割檔案時使用的檔案名稱前綴。例如，如果分割的檔案名為 `part.00000000000000000000000000000000`、`part.00000000000000000000000000000001` 等，則 `<prefix>` 應為 `part`。

**注意事項:**

* `myHeal` 程式依賴於 `myBreak` 程式產生的 meta 檔案 (`<prefix>.meta`)，該檔案包含了重建原始檔案所需的 chunk 數量和 chunk 大小資訊。
* 如果 meta 檔案遺失或損壞，`myHeal` 將無法正確重建原始檔案。
* 如果任何 chunk 檔案遺失，程式將會輸出錯誤訊息並刪除已建立的目标檔案。
* 如果 chunk 的實際大小與 meta 檔案中記錄的大小不符 (除了最後一個 chunk 可能較小之外)，程式將會輸出警告訊息，但仍會嘗試繼續組裝。這可能會導致組裝後的檔案不完整或損壞。

**Makefile (`Makefile`)**

```makefile
CC = gcc
CFLAGS = -Wall -O2
TARGET = myheal

$(TARGET): myHeal.c
	$(CC) $(CFLAGS) myHeal.c -o $(TARGET)

clean:
	rm -f $(TARGET)
```

**說明:**

* **`CC = gcc`**: 定義編譯器為 `gcc`。
* **`CFLAGS = -Wall -O2`**: 定義編譯選項。
    * `-Wall`: 開啟所有編譯警告。
    * `-O2`: 開啟第二級最佳化。
* **`TARGET = myheal`**: 定義目標執行檔名稱為 `myheal`。
* **`$(TARGET): myHeal.c`**: 指出目標 `myheal` 依賴於原始碼檔案 `myHeal.c`。
* **`$(CC) $(CFLAGS) myHeal.c -o $(TARGET)`**: 編譯 `myHeal.c` 並產生可執行檔 `myheal`。
* **`clean:`**: 定義一個名為 `clean` 的偽目標，用於清理編譯產生的檔案。
* **`rm -f $(TARGET)`**: 移除目標執行檔 `myheal`。

將 `myHeal.c`、`Makefile` 和 `README.md` 放在同一個目錄下，你就可以使用 `make` 命令來編譯你的 `myHeal` 程式了。
