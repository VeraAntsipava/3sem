extern "C"
void ReplaceMinMax(int* arr, int n, int minVal, int maxVal, int avg)
{
    for (int i = 0; i < n; i++)
        if (arr[i] == minVal || arr[i] == maxVal)
            arr[i] = avg;
}
