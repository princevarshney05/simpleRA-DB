#include "global.h"
/**
 * @brief 
 * SYNTAX: RENAME column_name TO column_name FROM relation_name
 */
bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR : 2 arguments expected" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    return true;
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");

    if (!matrixCatalogue.isMatrix(parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: MATRIX doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeTRANSPOSE()
{
    logger.log("executeTRANSPOSE");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.renameRelationName);
    return;
}