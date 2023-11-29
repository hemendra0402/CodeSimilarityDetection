#include <iostream>
#include <clang-c/Index.h>
#include <vector>
#include <string>
#include <iomanip>

// compile
// my clang file are in my C:\LLVM folder which is why i specify my libraries that way
// clang++ -I "C:\LLVM\include" -L "C:\LLVM\lib" main.cpp -o hello.exe -llibclang
// ./hello runs the program
// clang++ -std=c++11 -I/usr/local/opt/llvm/include -L/usr/local/opt/llvm/lib main.cpp -o hello.exe -lclang
// clang++ -std=c++14 -I "C:\LLVM\include" -L "C:\LLVM\lib" mymain.cpp -o hello.exe -llibclang


class Node {
    public: 
        Node(const std::string& functionName, const std::string& fileName) : functionName(functionName), fileName(fileName), numCallExpr(0), numParamDecl(0) {}
        
        static enum CXChildVisitResult Visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) 
        {
            Node* node = static_cast<Node*>(client_data);
            if (clang_getCursorKind(cursor) == CXCursor_CallExpr) 
            {
                node->numCallExpr++;
            } 
            else if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) 
            {
                node->numParamDecl++;
            }
            return CXChildVisit_Recurse;
        }

        void VisitChildren(CXCursor cursor) 
        {
            clang_visitChildren(cursor, Node::Visitor, this);
        }

        void PrintCounts() const 
        {
            std::cout << "File: " << fileName << std::endl;
            std::cout << "Function: " << functionName << std::endl;
            std::cout << "Number of CallExpr: " << numCallExpr << std::endl;
            std::cout << "Number of ParamDecl: " << numParamDecl << std::endl;
        }

        std::vector<int> GetValues() const
        {
            std::vector<int> values = {numCallExpr, numParamDecl};
            return values;
        }

        std::string GetFileName() const
        {
            return fileName;
        }

        std::string GetFunctionName() const
        {
            return functionName;
        }

    private:
        std::string functionName;
        std::string fileName;
        int numCallExpr;
        int numParamDecl;
};

Node createNode(CXCursor cursor, CXString functionName, CXString fileName)
{
    // Pass off the cursor to a different custom "visitor" of the Node class
    Node node(clang_getCString(functionName), clang_getCString(fileName));
    node.VisitChildren(cursor);

    return node;
}


// Generic visitor function (for Node finding, not for Node creation)
CXChildVisitResult generic_visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) {
    std::vector<Node>& nodeVector = *reinterpret_cast<std::vector<Node>*>(clientData);
    CXSourceLocation location = clang_getCursorLocation(cursor);

    if (clang_Location_isFromMainFile(location) == 0) {
        // Continues the cursor traversal with the next sibling of the cursor just 
        // visited, without visiting its children.
        return CXChildVisit_Continue;
    }

    CXCursorKind cursorKind = clang_getCursorKind(cursor);
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    CXString fileName;
    unsigned line, column;
    
    // get location relative to position in file
    clang_getPresumedLocation(location, &fileName, &line, &column);

    // Check if the cursor kind is a Class
    if (cursorKind == CXCursor_ClassDecl) {
        std::cout << "Class: " << clang_getCString(cursorSpelling) << std::endl;
        // // Perform additional actions here...
    }
    // Check if the cursor kind is a Function
    else if (cursorKind == CXCursor_FunctionDecl) {
        Node node = createNode(cursor, cursorSpelling, fileName);
        nodeVector.push_back(node);
        // Add node to a running list for this source code file, etc.
        // Do not recursively traverse (handled in Node creation)
        // Continues the cursor traversal with the next sibling of the cursor just 
        // visited, without visiting its children.
        return CXChildVisit_Continue; 
    }

    // clean up stuff for clang
    clang_disposeString(cursorSpelling);

    return CXChildVisit_Recurse; 
}

// not really used more for debugging
void printAST(CXCursor cursor, int level) {
    // get cursor info
    CXString cursorKind = clang_getCursorKindSpelling(clang_getCursorKind(cursor));
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXString fileName;
    unsigned line, column;
    
    // get location relative to position in file
    clang_getPresumedLocation(location, &fileName, &line, &column);

    // check if the location is in the file
    bool print;
    if (clang_Location_isFromMainFile(location) == 0) {
        print = false;
    }
    else
    {
        print = true;
    }

    // if the function is printable print it
    if (print)
    {
        for (int i = 0; i < level; ++i)
            std::cout << "  ";

        std::cout << "'" << clang_getCString(cursorSpelling) << "' (" << clang_getCString(cursorKind) << ") at " << clang_getCString(fileName) << ":" << line << ":" << column << std::endl;
    }

    // dispose of variables
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

// not really used but wanted to keep
void collectFunctionInfo(CXCursor cursor, std::vector<std::string>& functionInfo) { 
    // if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
    //     return;  
    
    CXSourceLocation location = clang_getCursorLocation(cursor);

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

        //collect info if in file
        std::string functionDetails = functionName + " | Return Type: " + returnTypeStr + " | " + paramInfo;
        bool print;
        if (clang_Location_isFromMainFile(location) != 0) {
            functionInfo.push_back(functionDetails);
        }
    }

    // traversing children
    clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
        collectFunctionInfo(c, *reinterpret_cast<std::vector<std::string>*>(client_data));
        return CXChildVisit_Continue;
    }, &functionInfo);
}


void ReadFile(const char* fileName, std::vector<Node>& nodeVector) 
{
    CXIndex index = clang_createIndex(0, 0);

    CXTranslationUnit translation = clang_parseTranslationUnit(
        index,
        fileName,
        nullptr,
        0,
        nullptr,
        0,
        CXTranslationUnit_None
    );

    if (!translation) {
        std::cerr << "Unable to parse translation unit for " << fileName << " Exiting function" << std::endl;
        clang_disposeTranslationUnit(translation);
        clang_disposeIndex(index);
        return;
    }

    // get the cursor
    CXCursor cursor = clang_getTranslationUnitCursor(translation);

    // start ast traversal
    printAST(cursor, 0);
    std::cout << std::endl;
    clang_visitChildren(clang_getTranslationUnitCursor(translation), generic_visitor, &nodeVector);

    // dispose translation unit and the index for it
    clang_disposeTranslationUnit(translation);
    clang_disposeIndex(index);
}

double SimilarityCalculation(const std::vector<int>& function1, const std::vector<int>& function2)
{
    double distance = 0.0;
    double maxdistance = 0.0;
    
    // Calculate Euclidean distance
    for (int i = 0; i < function1.size(); i++) 
    {
        double difference = function1[i] - function2[i];
        distance += pow(difference, 2);
        maxdistance += pow(std::max(function1[i], function2[i]), 2); // Calculate maximum possible distance
    }
    
    distance = sqrt(distance);
    maxdistance = sqrt(maxdistance);

    // Calculate similarity score as a percentage
    double similarity = ((maxdistance - distance) / maxdistance);
    return similarity;
}


int main() 
{
    const char *fileName1 = "file1.cpp";
    const char *fileName2 = "file2.cpp";
    std::vector<Node> file1Nodes;
    std::vector<Node> file2Nodes;
    ReadFile(fileName1, file1Nodes);    
    ReadFile(fileName2, file2Nodes);

    // columns
    std::cout << std::setw(15) << std::right << " "; // Align the first column header
    for (const auto& file2Node : file2Nodes) 
    {
        std::cout << std::setw(15) << std::right << file2Node.GetFunctionName();
    }
    std::cout << std::endl;

    // Print out function info with aligned columns
    for (const auto& file1Node : file1Nodes) 
    {
        std::vector<int> file1Vector = file1Node.GetValues();
        std::cout << std::setw(15) << std::right << file1Node.GetFunctionName();

        for (const auto& file2Node : file2Nodes) 
        {
            std::vector<int> file2Vector = file2Node.GetValues();
            double similarity = SimilarityCalculation(file1Vector, file2Vector);
            std::cout << std::setw(15) << std::right << similarity;
        }
        std::cout << std::endl;
    }
    return 0;
}
