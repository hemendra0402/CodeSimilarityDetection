Function Similarity Scorer for Source Code Files
This project aims to develop an algorithm for calculating similarity scores between functions in different source code files. The algorithm compares functions from one file against functions in another file and outputs a matrix of similarity scores for all pairings.

Goal
Input: Two separate source code files.
Output: Matrix displaying similarity scores between each function in one file against every function in the other file.
Implementation
The algorithm can be implemented in C/C++ or Java.
Develop function profiles based on attributes such as function name, return type, parameter count, etc., to establish the basis for similarity comparisons.
Justify and compare various approaches regarding their strengths and weaknesses. For instance, assess the feasibility of collecting function profiles and evaluate the speed of profile comparisons.
Conduct experimental testing to ensure correctness and measure the effectiveness of the implemented solution.
Explore the trade-off between abstraction and specificity when comparing two functions, ensuring the comparison is neither too abstract (reporting every function as similar) nor too narrow (reporting only exact copies as similar).
