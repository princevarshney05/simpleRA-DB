#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    parsedQuery.exportMatrixName = tokenizedQuery[1];
    return true;
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");
    //Table should exist
    if (matrixCatalogue.isMatrix(parsedQuery.exportMatrixName))
        return true;
    cout << "SEMANTIC ERROR: No such matrix exists" << endl;
    return false;
}

void executeTRANSPOSE()
{
    logger.log("executeTRANSPOSE");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.exportMatrixName);
    matrix->transpose();
    return;
}