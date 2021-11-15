#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[7] != "ON" || tokenizedQuery[11] != "BUFFER")
    {
        cout << "SYNTAX ERROR : Query Size, missing ON or BUFFER" << endl;
        return false;
    }
    if (tokenizedQuery[4] == "NESTED")
        parsedQuery.queryType = JOIN_NESTED;
    else if (tokenizedQuery[4] == "PARTHASH")
        parsedQuery.queryType = JOIN_PARTHASH;
    else
    {
        cout << "SYNTAX ERROR : INVALID JOIN ALGORITHM" << endl;
        return false;
    }

    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    parsedQuery.joinBuffer = stoi(tokenizedQuery[12]);

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeJOIN()
{
    logger.log("executeJOIN");
    Table *table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    Table *resultTable = new Table(parsedQuery.joinResultRelationName);
    int bufferSize = parsedQuery.joinBuffer;

    if (parsedQuery.queryType == JOIN_NESTED)
    {
        int maxBlocksTable1 = bufferSize - 2;
        int startPageIndex = 0, endPageIndex = maxBlocksTable1 - 1;
        vector<vector<int>> allRows;
        while (1)
        {

            if (endPageIndex > table1->blockCount - 1)
                endPageIndex = table1->blockCount - 1;

            allRows = table1->getRowsFromBlocks(startPageIndex, endPageIndex);

            //compare with rows of each block of table2 (1 block at a time)
            //for match conditions add that row into result table (1 block at a time)

            // -------------
            for (int i = 0; i < allRows.size(); i++)
            {
                for (int j = 0; j < allRows[i].size(); j++)
                    cout << allRows[i][j] << " ";
                cout << endl;
            }
            // -----------

            startPageIndex += maxBlocksTable1;
            endPageIndex += maxBlocksTable1;

            allRows.clear();
            if (startPageIndex >= table1->blockCount)
                break;
        }
    }
    else if (parsedQuery.queryType == JOIN_PARTHASH)
    {
    }
    return;
}