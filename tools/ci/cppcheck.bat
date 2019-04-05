ECHO ======== Check CppCheck Start ========

cppcheck --enable=warning,performance --inconclusive cpp_concurrency_in_action test > cppcheck.log

ECHO ======== Check CppCheck Result ========