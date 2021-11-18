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
    int bufferSize = parsedQuery.joinBuffer;
    int firstColumnIndex = table1->getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColumnIndex = table2->getColumnIndex(parsedQuery.joinSecondColumnName);

    vector<string> columns;
    for (int columnCounter = 0; columnCounter < table1->columnCount; columnCounter++)
        columns.emplace_back(table1->columns[columnCounter]);
    for (int columnCounter = 0; columnCounter < table2->columnCount; columnCounter++)
        columns.emplace_back(table2->columns[columnCounter]);

    Table *resultTable = new Table(parsedQuery.joinResultRelationName, columns);

    if (parsedQuery.queryType == JOIN_NESTED)
    {
        int maxBlocksTable1 = bufferSize - 2;
        int startPageIndex = 0, endPageIndex = maxBlocksTable1 - 1;
        vector<vector<int>> allRows, table2rows;

        ofstream fout(resultTable->sourceFileName, ios::app);

        while (1)
        {

            if (endPageIndex > table1->blockCount - 1)
                endPageIndex = table1->blockCount - 1;

            //get all rows from set of blocks from outer relation
            allRows = table1->getRowsFromBlocks(startPageIndex, endPageIndex);

            //compare with rows of each block of table2 (1 block at a time)
            //for match conditions add that row into result table (1 block at a time)
            // getting tuples from each block of table2:
            for (int blockNo = 0; blockNo < table2->blockCount; blockNo++)
            {
                table2rows = table2->blockRows(blockNo);
                for (int i = 0; i < allRows.size(); i++)
                    for (int j = 0; j < table2rows.size(); j++)
                        if (allRows[i][firstColumnIndex] == table2rows[j][secondColumnIndex])
                            resultTable->writeRow<int>(allRows[i], table2rows[j], fout);
                table2rows.clear();
            }

            startPageIndex += maxBlocksTable1;
            endPageIndex += maxBlocksTable1;

            table2rows.clear();
            allRows.clear();
            if (startPageIndex >= table1->blockCount)
                break;
        }
        fout.close();
        if (resultTable->blockify())
            tableCatalogue.insertTable(resultTable);
    }
    else if (parsedQuery.queryType == JOIN_PARTHASH)
    {

        int M = bufferSize - 1; // M : Bucket Size
        vector<vector<vector<int>>> hashedTable1(M);
        vector<int> BucketBlocks1(M, 0);          // no of blocks of each bucket
        vector<int> rowsInCurrentBucket1(M, 0);   //rows in current block of bucket
        vector<vector<int>> rowsInEachBucket1(M); //final no of rows in each bucket block
        set<int> bucketIds1;                      // list of bucket Ids

        // splitting Table1 into M buckets
        for (int blockNo = 0; blockNo < table1->blockCount; blockNo++)
        {
            vector<vector<int>> blockRows = table1->blockRows(blockNo);
            for (int rowNo = 0; rowNo < blockRows.size(); rowNo++)
            {
                int bucketId = blockRows[rowNo][firstColumnIndex] % M;
                bucketIds1.insert(bucketId);
                hashedTable1[bucketId].push_back(blockRows[rowNo]);
                rowsInCurrentBucket1[bucketId]++;
                if (rowsInCurrentBucket1[bucketId] == table1->maxRowsPerBlock)
                {
                    string BucketName = "Bkt_" + table1->tableName + "_" + to_string(bucketId);
                    bufferManager.writePage(BucketName, BucketBlocks1[bucketId], hashedTable1[bucketId], rowsInCurrentBucket1[bucketId]);
                    BucketBlocks1[bucketId]++;
                    rowsInCurrentBucket1[bucketId] = 0;
                    rowsInEachBucket1[bucketId].push_back(rowsInCurrentBucket1[bucketId]);
                    hashedTable1[bucketId].clear();
                }
            }
        }

        //writing left over blocks(with <maxRowsPerBlock) into buffer.
        for (auto bucketId : bucketIds1)
        {
            if (rowsInCurrentBucket1[bucketId] > 0)
            {
                string BucketName = "Bkt_" + table1->tableName + "_" + to_string(bucketId);
                bufferManager.writePage(BucketName, BucketBlocks1[bucketId], hashedTable1[bucketId], rowsInCurrentBucket1[bucketId]);
                BucketBlocks1[bucketId]++;
                rowsInCurrentBucket1[bucketId] = 0;
                rowsInEachBucket1[bucketId].push_back(rowsInCurrentBucket1[bucketId]);
                hashedTable1[bucketId].clear();
            }
        }

        hashedTable1.clear();

        vector<vector<vector<int>>> hashedTable2(M);
        vector<int> BucketBlocks2(M, 0);          // no of blocks of each bucket
        vector<int> rowsInCurrentBucket2(M, 0);   //rows in current block of bucket
        vector<vector<int>> rowsInEachBucket2(M); //final no of rows in each bucket block
        set<int> bucketIds2;                      // list of bucket Ids

        // splitting Table2 into M buckets
        for (int blockNo = 0; blockNo < table2->blockCount; blockNo++)
        {
            vector<vector<int>> blockRows = table2->blockRows(blockNo);
            for (int rowNo = 0; rowNo < blockRows.size(); rowNo++)
            {
                int bucketId = blockRows[rowNo][firstColumnIndex] % M;
                bucketIds2.insert(bucketId);
                hashedTable2[bucketId].push_back(blockRows[rowNo]);
                rowsInCurrentBucket2[bucketId]++;
                if (rowsInCurrentBucket2[bucketId] == table2->maxRowsPerBlock)
                {
                    string BucketName = "Bkt_" + table2->tableName + "_" + to_string(bucketId);
                    bufferManager.writePage(BucketName, BucketBlocks2[bucketId], hashedTable2[bucketId], rowsInCurrentBucket2[bucketId]);
                    BucketBlocks2[bucketId]++;
                    rowsInCurrentBucket2[bucketId] = 0;
                    rowsInEachBucket2[bucketId].push_back(rowsInCurrentBucket2[bucketId]);
                    hashedTable2[bucketId].clear();
                }
            }
        }

        //writing left over blocks(with <maxRowsPerBlock) into buffer.
        for (auto bucketId : bucketIds2)
        {
            if (rowsInCurrentBucket2[bucketId] > 0)
            {
                string BucketName = "Bkt_" + table2->tableName + "_" + to_string(bucketId);
                bufferManager.writePage(BucketName, BucketBlocks2[bucketId], hashedTable2[bucketId], rowsInCurrentBucket2[bucketId]);
                BucketBlocks2[bucketId]++;
                rowsInCurrentBucket2[bucketId] = 0;
                rowsInEachBucket2[bucketId].push_back(rowsInCurrentBucket2[bucketId]);
                hashedTable2[bucketId].clear();
            }
        }

        hashedTable2.clear();
    }
    return;
}