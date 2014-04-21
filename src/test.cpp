#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "myers.h"

using namespace std;


void runRandomTests(int numTests, int mode, bool findAlignment);
bool runTests();

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength, int mode, int* score, int* position);

bool checkAlignment(const unsigned char* query, int queryLength,
                    const unsigned char* target, int targetLength,
                    int score, int pos, int mode,
                    unsigned char* alignment, int alignmentLength);

int main() {
    srand(1);
    
    printf("Testing HW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_HW, true);
    printf("\n");

    printf("Testing HW...\n");
    runRandomTests(1000, MYERS_MODE_HW, false);
    printf("\n");
    
    printf("Testing NW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_NW, true);
    printf("\n");

    printf("Testing NW...\n");
    runRandomTests(1000, MYERS_MODE_NW, false);
    printf("\n");
      
    printf("Testing SHW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_SHW, true);
    printf("\n");
    
    printf("Testing SHW...\n");
    runRandomTests(1000, MYERS_MODE_SHW, false);
    printf("\n");
    
    printf("Specific tests:\n");
    if (runTests())
        printf("All specific tests passed!\n");
    else
        printf("Some specific tests failed\n");
    
    return 0;
}


int min(int x, int y) {
    return x < y ? x : y;
}

int min3(int x, int y, int z) {
    return min(x, min(y, z));
}

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength, int mode, int* score, int* position) {
    int* C = new int[queryLength];
    int* newC = new int[queryLength];

    int bestScore = -1;
    int pos = -1;

    // set first column (column zero)
    for (int i = 0; i < queryLength; i++)
        C[i] = i+1;
    /*
    for (int i = 0; i < queryLength; i++)
        printf("%3d ", C[i]);
    printf("\n");
    */
    for (int c = 0; c < targetLength; c++) { // for each column
        newC[0] = min3((mode == MYERS_MODE_HW ? 0 : c + 1) + 1, // up
                       (mode == MYERS_MODE_HW ? 0 : c) + (target[c] == query[0] ? 0 : 1), // up left
                       C[0] + 1); // left
        for (int r = 1; r < queryLength; r++) {
            newC[r] = min3(newC[r-1] + 1, // up
                           C[r-1] + (target[c] == query[r] ? 0 : 1), // up left
                           C[r] + 1); // left
        }
        
        /*  for (int i = 0; i < queryLength; i++)
            printf("%3d ", newC[i]);
            printf("\n");*/

        if (mode == MYERS_MODE_HW || mode == MYERS_MODE_SHW
            || c == targetLength - 1) // For NW check only last column
            if (bestScore == -1 || newC[queryLength-1] < bestScore) {
                bestScore = newC[queryLength-1];
                pos = c;
            }
        
        int *tmp = C;
        C = newC;
        newC = tmp;
    }

    delete[] C;
    delete[] newC;

    *score = bestScore;
    *position = pos;

    return MYERS_STATUS_OK;
}

void fillRandomly(unsigned char* seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = rand() % alphabetLength;
}

void runRandomTests(int numTests, int mode, bool findAlignment) {
    int alphabetLength = 10;
    int numTestsFailed = 0;
    clock_t start, finish;
    double timeMyers = 0;
    double timeSimple = 0;
    
    for (int i = 0; i < numTests; i++) {
        bool failed = false;
        int queryLength = 10 + rand() % 200;
        int targetLength = 100 + rand() % 20000;
        unsigned char query[queryLength];
        unsigned char target[targetLength];
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);
        /*
        // Print query
        printf("Query: ");
        for (int i = 0; i < queryLength; i++)
            printf("%d", query[i]);
        printf("\n");

        // Print target
        printf("Target: ");
        for (int i = 0; i < targetLength; i++)
            printf("%d", target[i]);
        printf("\n");
        */  
        start = clock();
        int score1; int pos1;
        unsigned char* alignment; int alignmentLength;
        myersCalcEditDistance(query, queryLength, target, targetLength,
                              alphabetLength, -1, mode, &score1, &pos1,
                              findAlignment, &alignment, &alignmentLength);
        timeMyers += clock() - start;
        if (alignment) {
            /*printf("Alignment: ");
            for (int i = 0; i < alignmentLength; i++)
                printf("%d", alignment[i]);
            printf("\n");
            printf("%d, %d\n", score1, pos1);*/
            if (!checkAlignment(query, queryLength, target, targetLength,
                                score1, pos1, mode, alignment, alignmentLength)) {
                failed = true;
                printf("Alignment is not correct\n");
            }
            free(alignment);
        }
        
        start = clock();
        int score2; int pos2;
        calcEditDistanceSimple(query, queryLength, target, targetLength,
                               alphabetLength, mode, &score2, &pos2);
        timeSimple += clock() - start;
        
        if (score1 != score2 || pos1 != pos2) {
            failed = true;
            printf("(%d, %d), (%d, %d)\n", score1, pos1, score2, pos2);
        }

        for (int k = score2 - 1; k <= score2 + 1; k++) {
            int score3, pos3;
            unsigned char* alignment3; int alignmentLength3;
            int scoreExpected = score2 > k ? -1 : score2;
            myersCalcEditDistance(query, queryLength, target, targetLength,
                                  alphabetLength, k, mode, &score3, &pos3,
                                  findAlignment, &alignment3, &alignmentLength3);
            if (score3 != scoreExpected ) {
                failed = true;
                printf("For k = %d score was %d but it should have been %d\n",
                       k, score3, scoreExpected);
            }
            if (alignment3) {
                if (!checkAlignment(query, queryLength, target, targetLength,
                                    score3, pos3, mode, alignment3, alignmentLength3)) {
                    failed = true;
                    printf("Alignment is not correct\n");
                }
                free(alignment3);
            }
        }

        if (failed)
            numTestsFailed++;
    }
    
    printf(mode == MYERS_MODE_HW ? "HW: " : mode == MYERS_MODE_SHW ? "SHW: " : "NW: ");
    printf(numTestsFailed == 0 ? "\x1B[32m" : "\x1B[31m");
    printf("%d/%d", numTests - numTestsFailed, numTests);
    printf("\x1B[0m");
    printf(" random tests passed!\n");
    double mTime = ((double)(timeMyers))/CLOCKS_PER_SEC;
    double sTime = ((double)(timeSimple))/CLOCKS_PER_SEC;
    printf("Time Myers: %lf\n", mTime);
    printf("Time Simple: %lf\n", sTime);
    printf("Times faster: %.2lf\n", sTime / mTime);
}


bool executeTest(const unsigned char* query, int queryLength,
                 const unsigned char* target, int targetLength,
                 int alphabetLength, int score, int pos, int mode) {
    int score1; int pos1;
    unsigned char* alignment; int alignmentLength;
    bool pass = true;
    myersCalcEditDistance(query, queryLength, target, targetLength,
                          alphabetLength, -1, mode, &score1, &pos1,
                          true, &alignment, &alignmentLength);

    int score2; int pos2;
    calcEditDistanceSimple(query, queryLength, target, targetLength,
                           alphabetLength, mode, &score2, &pos2);

    if (!(score1 == score2 && score2 == score && pos1 == pos2 && pos2 == pos)) {
        pass = false;
    }

    printf(mode == MYERS_MODE_HW ? "HW: " : mode == MYERS_MODE_SHW ? "SHW: " : "NW: ");
    printf("Myers -> (%d, %d), simple -> (%d, %d), correct -> (%d, %d)",
           score1, pos1, score2, pos2, score, pos);
    if (!checkAlignment(query, queryLength, target, targetLength,
                        score1, pos1, mode, alignment, alignmentLength)) {
        pass = false;
        printf("Alignment is not correct\n");
    }
    printf(pass ? "\x1B[32m OK \x1B[0m\n" : "\x1B[31m FAIL \x1B[0m\n");
    
    if (alignment) free(alignment);
    return pass;
}

bool test1() {
    int alphabetLength = 4;
    int queryLength = 4;
    int targetLength = 4;
    unsigned char query[4] = {0,1,2,3};
    unsigned char target[4] = {0,1,2,3};
    int scoreHW = 0;
    int scoreNW = 0;
    int scoreSHW = 0;
    int posHW = 3;
    int posNW = 3;
    int posSHW = 3;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool test2() {
    int alphabetLength = 9;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4}; // "match"
    unsigned char target[9] = {8,5,0,1,3,4,6,7,5}; // "remachine"
    int scoreHW = 1;
    int scoreNW = 6;
    int scoreSHW = 3;
    int posHW = 5;
    int posNW = 8;
    int posSHW = 5;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool test3() {
    int alphabetLength = 6;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4};
    unsigned char target[9] = {1,2,0,1,2,3,4,5,4};
    int scoreHW = 0;
    int scoreNW = 4;
    int scoreSHW = 2;
    int posHW = 6;
    int posNW = 8;
    int posSHW = 6;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool test4() {
    int alphabetLength = 2;
    int queryLength = 200;
    int targetLength = 200;
    unsigned char query[200] = {0};
    unsigned char target[200] = {1};
    int scoreHW = 1;
    int scoreNW = 1;
    int scoreSHW = 1;
    int posHW = 199;
    int posNW = 199;
    int posSHW = 199;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool test5() {
    int alphabetLength = 2;
    int queryLength = 64; // Testing for special case when queryLength == word size
    int targetLength = 64;
    unsigned char query[64] = {0};
    unsigned char target[64] = {1};
    int scoreHW = 1;
    int scoreNW = 1;
    int scoreSHW = 1;
    int posHW = 63;
    int posNW = 63;
    int posSHW = 63;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool test6() {
    int alphabetLength = 4;
    int queryLength = 13; // Testing for special case when queryLength == word size
    int targetLength = 420;
    unsigned char query[13] = {1,3,0,1,1,1,3,0,1,3,1,3,3};
    unsigned char target[420] = {0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,
                                 3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,
                                 1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,
                                 3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,
                                 3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,
                                 1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,
                                 3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,
                                 0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,
                                 2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,
                                 3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,
                                 3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,
                                 1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,
                                 2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,
                                 0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1};
    int scoreHW = 3;
    int scoreNW = 407;
    int scoreSHW = 4;
    int posHW = 31;
    int posNW = 419;
    int posSHW = 10;
    
    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, scoreHW, posHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreNW, posNW, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, scoreSHW, posSHW, MYERS_MODE_SHW);
    return r;
}

bool runTests() {
    bool t1 = test1();
    bool t2 = test2();
    bool t3 = test3();
    bool t4 = test4();
    bool t5 = test5();
    bool t6 = test6();
    return t1 && t2 && t3 && t4 && t5 && t6;
}

/**
 * Checks if alignment is correct.
 */
bool checkAlignment(const unsigned char* query, int queryLength,
                    const unsigned char* target, int targetLength,
                    int score, int pos, int mode,
                    unsigned char* alignment, int alignmentLength) {
    int alignScore = 0;
    int qIdx = queryLength - 1;
    int tIdx = pos;
    for (int i = alignmentLength - 1; i >= 0; i--) {
        if (alignment[i] == 0) { // (mis)match
            alignScore += query[qIdx] == target[tIdx] ? 0 : 1;
            qIdx--;
            tIdx--;
        }
        else if (alignment[i] == 1) {
            alignScore += 1;
            qIdx--;
        }
        else if (alignment[i] == 2) {
            if (!(mode == MYERS_MODE_HW && qIdx == -1))
                alignScore += 1;
            tIdx--;
        }
        if (tIdx < -1 || qIdx < -1) {
            printf("Alignment went outside of matrix! (tIdx, qIdx, i): (%d, %d, %d)\n", tIdx, qIdx, i);
            return false;
        }
    }
    if (qIdx != -1) {
        printf("Alignment did not reach end!\n");
        return false;
    }
    if (alignScore != score) {
        printf("Wrong score in alignment! %d should be %d\n", alignScore, score);
        return false;
    }
    return true;
}
