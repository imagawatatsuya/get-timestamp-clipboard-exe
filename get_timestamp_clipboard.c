// get_time_clipboard.c
// 目的: 日本の現在時刻を指定フォーマットで取得し、クリップボードにコピーする。
// 注意: このファイルは Shift_JIS (ANSI) で保存してください。

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h> // strcpy_s 用 (MinGWで使えない場合は要修正)

// --- 定数 ---
#define TIME_BUFFER_SIZE 80 // 時刻文字列に必要な十分なサイズ

// --- グローバル変数 (曜日用 - Shift_JISで保存) ---
const char* wday_jp[] = {"日", "月", "火", "水", "木", "金", "土"};

// --- プロトタイプ宣言 ---
int copy_to_clipboard(const char* text);

// --- メイン関数 ---
int main() {
    time_t now;
    struct tm local_time;
    char time_string[TIME_BUFFER_SIZE];
    int result_code = EXIT_SUCCESS; // 成功をデフォルトとする (stdlib.h)

    // --- 現在時刻取得 ---
    time(&now);
    if (localtime_s(&local_time, &now) != 0) {
        perror("Error getting local time"); // 標準エラー出力にエラーメッセージ
        return EXIT_FAILURE; // 異常終了
    }

    // --- 時刻文字列フォーマット ---
    // YYYY/MM/DD(曜) HH:MM:SS
    int written = snprintf(time_string, sizeof(time_string),
                           "%04d/%02d/%02d(%s) %02d:%02d:%02d",
                           local_time.tm_year + 1900,
                           local_time.tm_mon + 1,
                           local_time.tm_mday,
                           wday_jp[local_time.tm_wday], // Shift_JISで保存された曜日文字列
                           local_time.tm_hour,
                           local_time.tm_min,
                           local_time.tm_sec);

    // snprintf のエラーチェック (書き込み失敗またはバッファ不足による切り詰め)
    if (written < 0 || (size_t)written >= sizeof(time_string)) {
        fprintf(stderr, "Error formatting time string (written=%d, buffer_size=%zu)\n",
                written, sizeof(time_string));
        return EXIT_FAILURE; // 異常終了
    }

    printf("現在時刻: %s\n", time_string); // 確認用にコンソール表示

    // --- クリップボードにコピー ---
    if (copy_to_clipboard(time_string) != 0) {
        fprintf(stderr, "クリップボードへのコピーに失敗しました。\n");
        result_code = EXIT_FAILURE; // 異常終了
    } else {
        printf("時刻をクリップボードにコピーしました。\n");
    }

    return result_code; // 正常終了 (0) または異常終了 (1)
}

// --- クリップボードコピー関数 ---
// (基本的なロジックは変更なし、エラーメッセージを stderr に出力)
int copy_to_clipboard(const char* text) {
    HGLOBAL hGlobal = NULL; // NULLで初期化
    char* pGlobal = NULL;
    size_t text_len = strlen(text);
    BOOL opened_clipboard = FALSE; // クリップボードを開いたかどうかのフラグ

    // 1. クリップボードを開く
    if (!OpenClipboard(NULL)) {
        fprintf(stderr, "Error: Cannot open clipboard (Error code: %lu)\n", GetLastError());
        goto error_exit; // エラー処理へ
    }
    opened_clipboard = TRUE; // 開いたフラグを立てる

    // 2. クリップボードを空にする
    if (!EmptyClipboard()) {
        fprintf(stderr, "Error: Cannot empty clipboard (Error code: %lu)\n", GetLastError());
        goto error_exit; // エラー処理へ
    }

    // 3. グローバルメモリを確保
    // GMEM_MOVEABLE: メモリが移動可能, GMEM_ZEROINIT: ゼロクリア (必須ではないがあると安全)
    hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, text_len + 1); // NULL終端文字分+1
    if (hGlobal == NULL) {
        fprintf(stderr, "Error: GlobalAlloc failed (Error code: %lu)\n", GetLastError());
        goto error_exit; // エラー処理へ
    }

    // 4. メモリをロックしてポインタ取得
    pGlobal = (char*)GlobalLock(hGlobal);
    if (pGlobal == NULL) {
        fprintf(stderr, "Error: GlobalLock failed (Error code: %lu)\n", GetLastError());
        // GlobalLock失敗時は GlobalFree で解放する
        GlobalFree(hGlobal);
        hGlobal = NULL; // 解放したことを示す
        goto error_exit; // エラー処理へ
    }

    // 5. メモリに文字列をコピー (安全な関数を使用)
    // strcpy_s が MinGW で使えない場合は、strncpy と手動 NULL 終端に置き換え検討
    if (strcpy_s(pGlobal, text_len + 1, text) != 0) {
       fprintf(stderr, "Error: strcpy_s failed copying to clipboard memory\n");
       GlobalUnlock(hGlobal); // ロック解除
       GlobalFree(hGlobal);   // メモリ解放
       hGlobal = NULL;
       goto error_exit;      // エラー処理へ
    }

    // 6. メモリのロックを解除
    GlobalUnlock(hGlobal);
    pGlobal = NULL; // ポインタは無効になる

    // 7. クリップボードにデータを設定
    // CF_TEXT: ANSI テキスト形式 (Shift_JIS想定)
    // SetClipboardData成功後は、システムがメモリ(hGlobal)を管理するので解放しないこと
    if (SetClipboardData(CF_TEXT, hGlobal) == NULL) {
        fprintf(stderr, "Error: SetClipboardData failed (Error code: %lu)\n", GetLastError());
        // SetClipboardData が失敗した場合、確保したメモリは解放する必要がある
        GlobalFree(hGlobal);
        hGlobal = NULL;
        goto error_exit; // エラー処理へ
    }
    // ここまで来たら成功。hGlobalの所有権はシステムに移る

    // 8. クリップボードを閉じる
    CloseClipboard();
    opened_clipboard = FALSE; // 閉じたフラグ

    return 0; // 成功

error_exit:
    // エラー発生時の後始末
    if (opened_clipboard) {
        CloseClipboard(); // クリップボードが開いていれば閉じる
    }
    // hGlobal が確保されたままエラーになった場合 (SetClipboardData前) は解放するが、
    // 上記コードでは各エラー箇所で適切に解放 or 所有権移譲されているはず
    // if (hGlobal != NULL) {
    //     GlobalFree(hGlobal); // 不要なはず
    // }
    return 1; // 失敗
}