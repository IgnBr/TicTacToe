#include<iostream>
using namespace std;
#include "dllheader.h"
namespace dllspec{
    int dllclass::GetWinner(int wins[3], int gameBoard[9]){
        int cells[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 3, 6, 1, 4, 7, 2, 5, 8, 0, 4, 8, 2, 4, 6 };

        for(int i = 0; i < 24; i += 3)
        {
            if(gameBoard[cells[i]] == gameBoard[cells[i+1]] && gameBoard[cells[i]] == gameBoard[cells[i+2]])
            {
                wins[0] = cells[i];
                wins[1] = cells[i+1];
                wins[2] = cells[i+2];

                return gameBoard[cells[i]];
            }
        }

        for(int i = 0; i < 9; ++i)
        {
            if(gameBoard[i] == 0)
                return 0;
        }

        return 3;
    }

    void dllclass::ConvertIntArrayToString(int arr[], int n, char* outStr){
        for (int i = 0; i < n; ++i)
        {
            char n_char[1 + sizeof(char)];
            std::sprintf(n_char, "%d", arr[i]);
            outStr[i]=n_char[0];
        }
    }
}
