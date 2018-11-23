@ECHO OFF

cppcheck --enable=warning,performance --inconclusive ^
         -I cpp_primer5/ -I include/ ^
         --template=vs ^
         -q cpp_primer5 ^
         2>cppcheck.log

cppcheck --enable=warning,performance --inconclusive ^
         -I cpp_primer5/ ^
         --template=vs ^
         -q cpp_primer5 ^
         2>>cppcheck.log

ECHO ======== Check CppCheck Result ========

for /f %%C in ('Find /V /C "" ^< cppcheck.log') do set Count=%%C
echo Error Count: %Count%

if "%Count%" == "0" (GOTO CHECK_OK)

ECHO ======== CppCheck Result: FAIL ========
EXIT /b 1

:CHECK_OK
ECHO ======== CppCheck Result: OK   ========
EXIT /b 0

