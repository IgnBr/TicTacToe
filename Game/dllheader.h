#ifndef dllheader_H
#define dllheader_H
namespace dllspec{
    class dllclass {
    public:
        static __declspec(dllexport) int GetWinner(int wins[3], int gameBoard[9]);
        static __declspec(dllexport) void ConvertIntArrayToString(int arr[], int n, char* outStr);
    };
}
#endif // dllheader_H
