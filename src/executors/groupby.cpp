#include "global.h"

typedef map<int,int> groupByHash;
/**
 * @brief 
 * SYNTAX: R <- SELECT column_name bin_op [column_name | int_literal] FROM relation_name
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }

    string funAndColumn = tokenizedQuery[8];
    if(funAndColumn.length() >= 6){
        string groupingFunction = funAndColumn.substr(0,3);
        if(groupingFunction == "MAX")
            parsedQuery.groupingFunction = MAX;
        else if(groupingFunction == "MIN")
            parsedQuery.groupingFunction = MIN;
        else if(groupingFunction == "SUM")
            parsedQuery.groupingFunction = SUM;
        else if(groupingFunction == "AVG")
            parsedQuery.groupingFunction = AVG;
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }

        if(funAndColumn[3] == '(' and funAndColumn[funAndColumn.length()-1] == ')'){
            int columnNameLength = (funAndColumn.length()-2)-4+1;
            parsedQuery.groupbyFunAttribute = funAndColumn.substr(4,columnNameLength);
        }
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    
    

    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupbyResultRelationName = tokenizedQuery[0];
    parsedQuery.groupByAttribute = tokenizedQuery[4];
    parsedQuery.groupbyRelationName = tokenizedQuery[6];
    return true;
}

bool semanticParseGROUPBY(){
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupbyResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByAttribute, parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Grouping attribute doesn't exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupbyFunAttribute, parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Function attribute doesn't exist in relation" << endl;
        return false;
    }
    
    return true;
}

string EnumToSTring(GroupingFunction groupingFunction){
    switch (groupingFunction)
    {
        case MAX: return "MAX";
        case MIN: return "MIN";
        case SUM: return "SUM";
        case AVG: return "AVG";
    }
}

int solve(GroupingFunction groupingFunction,int oldValue, int currentValue){
    switch (groupingFunction)
    {
        case MAX: return max(oldValue,currentValue);
        case MIN: return min(oldValue,currentValue);
        case SUM: return oldValue + currentValue;
        case AVG: return oldValue + currentValue;
    }
}

int giveDefaultValue(GroupingFunction groupingFunction){
    switch (groupingFunction)
    {
        case MAX: return INT_MIN;
        case MIN: return INT_MAX;
        case SUM: return 0;
        case AVG: return 0;
    }
}

int findValue(GroupingFunction groupingFunction,groupByHash hs,int key){
    auto itr = hs.find(key);
    if(itr != hs.end()) return hs[key];
    else return giveDefaultValue(groupingFunction);
}

void executeGROUPBY(){
    logger.log("executeGROUPBY");

    Table table = *tableCatalogue.getTable(parsedQuery.groupbyRelationName);
    string col1 = parsedQuery.groupByAttribute;
    string col2 = EnumToSTring(parsedQuery.groupingFunction) + parsedQuery.groupbyFunAttribute;
    vector<string> resultantColumns = {col1,col2};
    Table* resultantTable = new Table(parsedQuery.groupbyResultRelationName, resultantColumns);
    Cursor cursor = table.getCursor();
    vector<int> row = cursor.getNext();
    int groupByColumnIndex = table.getColumnIndex(parsedQuery.groupByAttribute);
    int funColumnIndex = table.getColumnIndex(parsedQuery.groupbyFunAttribute);
    
    groupByHash groupByResults;
    groupByHash groupByLen;
    GroupingFunction gf = parsedQuery.groupingFunction;
    while (!row.empty())
    {

        int value1 = row[groupByColumnIndex];
        int value2 = row[funColumnIndex];
        
        groupByLen[value1] += 1;
        
        int oldValue = findValue(gf,groupByResults,value1);

        groupByResults[value1] = solve(gf,oldValue,value2);
            
        row = cursor.getNext();
    }
    for(auto &it: groupByResults){
        int n;
        if(gf == AVG){
            n = groupByLen[it.first];
        }
        else{
            n = 1;
        }
        row = {it.first,it.second/n};
        resultantTable->writeRow<int>(row);
    }
    
    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);
    else{
        cout<<"Empty Table"<<endl;
        resultantTable->unload();
        delete resultantTable;
    }
    return;
}