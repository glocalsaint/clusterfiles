#include <iostream>
#include <thread>
#include <unistd.h>

unsigned long long getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
} 
int main() {
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";
	for(int i=0;i<1000;i++)
		int *p = new int[1024*1024];
    std::cout << getTotalSystemMemory()/(1024*1024*1024) << "gb amount of memory available.\n";
//}
}
