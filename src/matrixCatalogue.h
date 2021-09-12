#include "matrix.h"

/**
 * @brief The MatrixCatalogue acts like an index of matrix existing in the
 * system. Everytime a matrix is added(or removed) to(or from) the system, 
 * it needs to be added(removed) to(from) the MatrixCatalogue. 
 *
 */
class MatrixCatalogue
{

    unordered_map<string, Matrix *> matrices;

public:
    MatrixCatalogue() {}
    void insertMatrix(Matrix *matrix);
    void deleteMatrix(string matrixName);
    Matrix *getMatrix(string matrixName);
    bool isMatrix(string matrixName);
    bool isColumnFromMatrix(string columnName, string matrixName);
    void print();
    ~MatrixCatalogue();
};