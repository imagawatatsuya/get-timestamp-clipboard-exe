@echo off
chcp 65001 > nul

set SRC_FILE=get_timestamp_clipboard.c
set EXE_FILE=get_timestamp_clipboard.exe
:: set LOG_FILE=execution_log.txt  <-- �폜

echo --- C����v���O�����̃R���p�C�� (%SRC_FILE%) ---
gcc "%SRC_FILE%" -o "%EXE_FILE%" -Wall -Wextra -O2 -luser32

if %errorlevel% neq 0 (
    echo !!! �R���p�C���Ɏ��s���܂����B!!!
    goto end
)
echo �R���p�C������: %EXE_FILE% �𐶐����܂����B
echo.

echo --- �v���O�����̎��s (%EXE_FILE%) ---
"%EXE_FILE%"

if %errorlevel% neq 0 (
    echo !!! �v���O�����̎��s���ɃG���[���������܂����B!!!
) else (
    echo �v���O�����͐���Ɋ������܂����B
    echo ���ݎ������N���b�v�{�[�h�ɃR�s�[����܂��� (Ctrl+V�œ\��t���m�F)�B
)
:: echo ���s���O�� %LOG_FILE% ���m�F���Ă��������B <-- �폜
echo.

:end
echo --- �����I�� ---
pause