#include "semanticParser.h"

void executeCommand();

void executeCLEAR();
void executeCROSS();
void executeDISTINCT();
void executeEXPORT();
void executeEXPORT_MATRIX();
void executeINDEX();
void executeJOIN();
void executeLIST();
void executeLOAD();
void executeLOAD_MATRIX();
void executePRINT();
void executePRINT_MATRIX();
void executeTRANSPOSE();
void executePROJECTION();
void executeRENAME();
void executeSELECTION();
void executeSORT();
void executeSOURCE();


bool evaluateBinOp(int value1, int value2, BinaryOperator binaryOperator);
void printRowCount(int rowCount);