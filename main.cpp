#include <iostream>
#include <clang-c/Index.h>

// compile
// my clang file are in my C:\LLVM folder which is why i specify my libraries that way
// clang++ -I "C:\LLVM\include" -L "C:\LLVM\lib" main.cpp -o hello.exe -llibclang
// ./hello runs the program
// clang++ -std=c++11 -I/usr/local/opt/llvm/include -L/usr/local/opt/llvm/lib main.cpp -o hello.exe -lclang
void printAST(CXCursor cursor, int level) {
    CXString cursorKind = clang_getCursorKindSpelling(clang_getCursorKind(cursor));
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXString fileName;
    unsigned line, column;
    clang_getPresumedLocation(location, &fileName, &line, &column);

    for (int i = 0; i < level; ++i)
        std::cout << "  ";

    std::cout << "Cursor '" << clang_getCString(cursorSpelling) << "' (" << clang_getCString(cursorKind) << ") at " << clang_getCString(fileName) << ":" << line << ":" << column << std::endl;

    clang_disposeString(cursorSpelling);
    clang_disposeString(fileName);
    clang_disposeString(cursorKind);

    // Using clang_visitChildren to iterate over children
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            printAST(c, *reinterpret_cast<int *>(client_data) + 1);
            return CXChildVisit_Continue;
        },
        &level
    );
}

int main() {
    CXIndex index = clang_createIndex(0, 0);
    printf("Hello\n");
    const char *fileName = "file1.cpp";
    const char *args[] = {"-std=c++11"};

    CXTranslationUnit translationUnit = clang_parseTranslationUnit(
        index,
        fileName,
        args,
        sizeof(args) / sizeof(*args),
        nullptr,
        0,
        CXTranslationUnit_None
    );
    
    if (!translationUnit) {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        return 1;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(translationUnit);
    printAST(cursor, 0);

    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);

    return 0;
}