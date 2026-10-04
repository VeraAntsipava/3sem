#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include <iostream>
using namespace std;

struct Data {
    int* arr;
    int n;
    int minVal;
    int maxVal;
    double avg;
};

typedef void (*ReplaceFunc)(int*, int, int, int, int);

void* MinMax(void* param)
{
    Data* d = (Data*)param;
    d->minVal = d->maxVal = d->arr[0];
    for (int i = 1; i < d->n; i++) {
        if (d->arr[i] < d->minVal) d->minVal = d->arr[i];
        usleep(7000);
        if (d->arr[i] > d->maxVal) d->maxVal = d->arr[i];
        usleep(7000);
    }
    cout << "min = " << d->minVal << ", max = " << d->maxVal << endl;
    return NULL;
}

void* Average(void* param)
{
    Data* d = (Data*)param;
    double sum = 0;
    for (int i = 0; i < d->n; i++) {
        sum += d->arr[i];
        usleep(12000);
    }
    d->avg = sum / d->n;
    cout << "average = " << d->avg << endl;
    return NULL;
}

int main()
{
    Data d;
    cout << "Array size: ";
    cin >> d.n;
    if (d.n <= 0) return 1;

    d.arr = new int[d.n];
    cout << "Elements: ";
    for (int i = 0; i < d.n; i++) cin >> d.arr[i];

    pthread_t hMinMax, hAverage;
    pthread_create(&hMinMax, NULL, MinMax, &d);
    pthread_create(&hAverage, NULL, Average, &d);

    pthread_join(hMinMax, NULL);
    pthread_join(hAverage, NULL);

    void* hLib = dlopen("./libhelper.dylib", RTLD_NOW);
    if (!hLib) {
        cerr << "Cannot load library: " << dlerror() << endl;
        return 1;
    }
    ReplaceFunc replace = (ReplaceFunc)dlsym(hLib, "ReplaceMinMax");
    if (replace)
        replace(d.arr, d.n, d.minVal, d.maxVal, (int)d.avg);
    dlclose(hLib);

    cout << "Result: ";
    for (int i = 0; i < d.n; i++) cout << d.arr[i] << " ";
    cout << endl;

    delete[] d.arr;
    return 0;
}