@echo off
chcp 65001 > nul

set SRC_FILE=get_timestamp_clipboard.c
set EXE_FILE=get_timestamp_clipboard.exe
:: set LOG_FILE=execution_log.txt  <-- 削除

echo --- C言語プログラムのコンパイル (%SRC_FILE%) ---
gcc "%SRC_FILE%" -o "%EXE_FILE%" -Wall -Wextra -O2 -luser32

if %errorlevel% neq 0 (
    echo !!! コンパイルに失敗しました。!!!
    goto end
)
echo コンパイル成功: %EXE_FILE% を生成しました。
echo.

echo --- プログラムの実行 (%EXE_FILE%) ---
"%EXE_FILE%"

if %errorlevel% neq 0 (
    echo !!! プログラムの実行中にエラーが発生しました。!!!
) else (
    echo プログラムは正常に完了しました。
    echo 現在時刻がクリップボードにコピーされました (Ctrl+Vで貼り付け確認)。
)
:: echo 実行ログは %LOG_FILE% を確認してください。 <-- 削除
echo.

:end
echo --- 処理終了 ---
pause