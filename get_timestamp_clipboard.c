// get_time_clipboard.c
// �ړI: ���{�̌��ݎ������w��t�H�[�}�b�g�Ŏ擾���A�N���b�v�{�[�h�ɃR�s�[����B
// ����: ���̃t�@�C���� Shift_JIS (ANSI) �ŕۑ����Ă��������B

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h> // strcpy_s �p (MinGW�Ŏg���Ȃ��ꍇ�͗v�C��)

// --- �萔 ---
#define TIME_BUFFER_SIZE 80 // ����������ɕK�v�ȏ\���ȃT�C�Y

// --- �O���[�o���ϐ� (�j���p - Shift_JIS�ŕۑ�) ---
const char* wday_jp[] = {"��", "��", "��", "��", "��", "��", "�y"};

// --- �v���g�^�C�v�錾 ---
int copy_to_clipboard(const char* text);

// --- ���C���֐� ---
int main() {
    time_t now;
    struct tm local_time;
    char time_string[TIME_BUFFER_SIZE];
    int result_code = EXIT_SUCCESS; // �������f�t�H���g�Ƃ��� (stdlib.h)

    // --- ���ݎ����擾 ---
    time(&now);
    if (localtime_s(&local_time, &now) != 0) {
        perror("Error getting local time"); // �W���G���[�o�͂ɃG���[���b�Z�[�W
        return EXIT_FAILURE; // �ُ�I��
    }

    // --- ����������t�H�[�}�b�g ---
    // YYYY/MM/DD(�j) HH:MM:SS
    int written = snprintf(time_string, sizeof(time_string),
                           "%04d/%02d/%02d(%s) %02d:%02d:%02d",
                           local_time.tm_year + 1900,
                           local_time.tm_mon + 1,
                           local_time.tm_mday,
                           wday_jp[local_time.tm_wday], // Shift_JIS�ŕۑ����ꂽ�j��������
                           local_time.tm_hour,
                           local_time.tm_min,
                           local_time.tm_sec);

    // snprintf �̃G���[�`�F�b�N (�������ݎ��s�܂��̓o�b�t�@�s���ɂ��؂�l��)
    if (written < 0 || (size_t)written >= sizeof(time_string)) {
        fprintf(stderr, "Error formatting time string (written=%d, buffer_size=%zu)\n",
                written, sizeof(time_string));
        return EXIT_FAILURE; // �ُ�I��
    }

    printf("���ݎ���: %s\n", time_string); // �m�F�p�ɃR���\�[���\��

    // --- �N���b�v�{�[�h�ɃR�s�[ ---
    if (copy_to_clipboard(time_string) != 0) {
        fprintf(stderr, "�N���b�v�{�[�h�ւ̃R�s�[�Ɏ��s���܂����B\n");
        result_code = EXIT_FAILURE; // �ُ�I��
    } else {
        printf("�������N���b�v�{�[�h�ɃR�s�[���܂����B\n");
    }

    return result_code; // ����I�� (0) �܂��ُ͈�I�� (1)
}

// --- �N���b�v�{�[�h�R�s�[�֐� ---
// (��{�I�ȃ��W�b�N�͕ύX�Ȃ��A�G���[���b�Z�[�W�� stderr �ɏo��)
int copy_to_clipboard(const char* text) {
    HGLOBAL hGlobal = NULL; // NULL�ŏ�����
    char* pGlobal = NULL;
    size_t text_len = strlen(text);
    BOOL opened_clipboard = FALSE; // �N���b�v�{�[�h���J�������ǂ����̃t���O

    // 1. �N���b�v�{�[�h���J��
    if (!OpenClipboard(NULL)) {
        fprintf(stderr, "Error: Cannot open clipboard (Error code: %lu)\n", GetLastError());
        goto error_exit; // �G���[������
    }
    opened_clipboard = TRUE; // �J�����t���O�𗧂Ă�

    // 2. �N���b�v�{�[�h����ɂ���
    if (!EmptyClipboard()) {
        fprintf(stderr, "Error: Cannot empty clipboard (Error code: %lu)\n", GetLastError());
        goto error_exit; // �G���[������
    }

    // 3. �O���[�o�����������m��
    // GMEM_MOVEABLE: ���������ړ��\, GMEM_ZEROINIT: �[���N���A (�K�{�ł͂Ȃ�������ƈ��S)
    hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, text_len + 1); // NULL�I�[������+1
    if (hGlobal == NULL) {
        fprintf(stderr, "Error: GlobalAlloc failed (Error code: %lu)\n", GetLastError());
        goto error_exit; // �G���[������
    }

    // 4. �����������b�N���ă|�C���^�擾
    pGlobal = (char*)GlobalLock(hGlobal);
    if (pGlobal == NULL) {
        fprintf(stderr, "Error: GlobalLock failed (Error code: %lu)\n", GetLastError());
        // GlobalLock���s���� GlobalFree �ŉ������
        GlobalFree(hGlobal);
        hGlobal = NULL; // ����������Ƃ�����
        goto error_exit; // �G���[������
    }

    // 5. �������ɕ�������R�s�[ (���S�Ȋ֐����g�p)
    // strcpy_s �� MinGW �Ŏg���Ȃ��ꍇ�́Astrncpy �Ǝ蓮 NULL �I�[�ɒu����������
    if (strcpy_s(pGlobal, text_len + 1, text) != 0) {
       fprintf(stderr, "Error: strcpy_s failed copying to clipboard memory\n");
       GlobalUnlock(hGlobal); // ���b�N����
       GlobalFree(hGlobal);   // ���������
       hGlobal = NULL;
       goto error_exit;      // �G���[������
    }

    // 6. �������̃��b�N������
    GlobalUnlock(hGlobal);
    pGlobal = NULL; // �|�C���^�͖����ɂȂ�

    // 7. �N���b�v�{�[�h�Ƀf�[�^��ݒ�
    // CF_TEXT: ANSI �e�L�X�g�`�� (Shift_JIS�z��)
    // SetClipboardData������́A�V�X�e����������(hGlobal)���Ǘ�����̂ŉ�����Ȃ�����
    if (SetClipboardData(CF_TEXT, hGlobal) == NULL) {
        fprintf(stderr, "Error: SetClipboardData failed (Error code: %lu)\n", GetLastError());
        // SetClipboardData �����s�����ꍇ�A�m�ۂ����������͉������K�v������
        GlobalFree(hGlobal);
        hGlobal = NULL;
        goto error_exit; // �G���[������
    }
    // �����܂ŗ����琬���BhGlobal�̏��L���̓V�X�e���Ɉڂ�

    // 8. �N���b�v�{�[�h�����
    CloseClipboard();
    opened_clipboard = FALSE; // �����t���O

    return 0; // ����

error_exit:
    // �G���[�������̌�n��
    if (opened_clipboard) {
        CloseClipboard(); // �N���b�v�{�[�h���J���Ă���Ε���
    }
    // hGlobal ���m�ۂ��ꂽ�܂܃G���[�ɂȂ����ꍇ (SetClipboardData�O) �͉�����邪�A
    // ��L�R�[�h�ł͊e�G���[�ӏ��œK�؂ɉ�� or ���L���ڏ�����Ă���͂�
    // if (hGlobal != NULL) {
    //     GlobalFree(hGlobal); // �s�v�Ȃ͂�
    // }
    return 1; // ���s
}