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

void collectFunctionInfo(CXCursor cursor, std::vector<std::string>& functionInfo) {
    // if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
    //     return;  

    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
        // get function name
        CXString cursorSpelling = clang_getCursorSpelling(cursor);
        std::string functionName = clang_getCString(cursorSpelling);
        clang_disposeString(cursorSpelling);

        // get func return type
        CXType returnType = clang_getCursorResultType(cursor);
        CXString returnTypeSpelling = clang_getTypeSpelling(returnType);
        std::string returnTypeStr = clang_getCString(returnTypeSpelling);
        clang_disposeString(returnTypeSpelling);

        // get parameter count and types
        unsigned int paramCount = clang_Cursor_getNumArguments(cursor);
        std::string paramInfo = "Params: ";
        for (unsigned int i = 0; i < paramCount; ++i) {
            CXCursor paramCursor = clang_Cursor_getArgument(cursor, i);
            CXString paramTypeSpelling = clang_getTypeSpelling(clang_getCursorType(paramCursor));
            std::string paramTypeStr = clang_getCString(paramTypeSpelling);
            clang_disposeString(paramTypeSpelling);
            paramInfo += paramTypeStr + " ";
        }

        //collect
        std::string functionDetails = functionName + " | Return Type: " + returnTypeStr + " | " + paramInfo;
        functionInfo.push_back(functionDetails);
    }

    // traversing children
    clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
        collectFunctionInfo(c, *reinterpret_cast<std::vector<std::string>*>(client_data));
        return CXChildVisit_Continue;
    }, &functionInfo);
}

int main() {
    CXIndex index = clang_createIndex(0, 0);
    
    // Parse first file (file1.cpp)
    const char *fileName1 = "file1.cpp";
    const char *args1[] = {"-std=c++11"};
    CXTranslationUnit translationUnit1 = clang_parseTranslationUnit(
        index,
        fileName1,
        args1,
        sizeof(args1) / sizeof(*args1),
        nullptr,
        0,
        CXTranslationUnit_None
    );
    
    if (!translationUnit1) {
        std::cerr << "Unable to parse translation unit for file1. Quitting." << std::endl;
        clang_disposeIndex(index);
        return 1;
    }

    // Parse second file (file2.cpp)
    const char *fileName2 = "file2.cpp";
    const char *args2[] = {"-std=c++11"};
    CXTranslationUnit translationUnit2 = clang_parseTranslationUnit(
        index,
        fileName2,
        args2,
        sizeof(args2) / sizeof(*args2),
        nullptr,
        0,
        CXTranslationUnit_None
    );
    
    if (!translationUnit2) {
        std::cerr << "Unable to parse translation unit for file2. Quitting." << std::endl;
        clang_disposeTranslationUnit(translationUnit1);
        clang_disposeIndex(index);
        return 1;
    }

    // Get cursors for both ASTs
    CXCursor cursor1 = clang_getTranslationUnitCursor(translationUnit1);
    CXCursor cursor2 = clang_getTranslationUnitCursor(translationUnit2);
    printAST(cursor1, 0);
    printAST(cursor2, 0);

    // Collect function information for file1.cpp
    std::vector<std::string> functionInfo1;
    collectFunctionInfo(cursor1, functionInfo1);

    // Collect function information for file2.cpp
    std::vector<std::string> functionInfo2;
    collectFunctionInfo(cursor2, functionInfo2);

    // Print function information for file1.cpp
    std::cout << "Functions in file1.cpp:" << std::endl;
    for (const auto& info : functionInfo1) {
        std::cout << info << std::endl;
    }

    // Print function information for file2.cpp
    std::cout << "Functions in file2.cpp:" << std::endl;
    for (const auto& info : functionInfo2) {
        std::cout << info << std::endl;
    }
    // Dispose Translation Units and Index
    clang_disposeTranslationUnit(translationUnit1);
    clang_disposeTranslationUnit(translationUnit2);
    clang_disposeIndex(index);

    return 0;
}