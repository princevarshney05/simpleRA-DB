#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT_MATRIX()
{
    logger.log("syntacticParseEXPORT_MATRIX");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT_MATRIX;
    parsedQuery.exportMatrixName = tokenizedQuery[2];
    return true;
}

bool semanticParseEXPORT_MATRIX()
{
    logger.log("semanticParseEXPORT_MATRIX");
    //Table should exist
    if (matrixCatalogue.isMatrix(parsedQuery.exportMatrixName))
        return true;
    cout << "SEMANTIC ERROR: No such matrix exists" << endl;
    return false;
}

void executeEXPORT_MATRIX()
{
    logger.log("executeEXPORT_MATRIX");
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.exportMatrixName);
    matrix->makePermanent();
    return;
}