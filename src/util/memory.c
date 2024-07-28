#include "memory.h"

void humanSize(u64 bytes, char** suffix, u32* whole, u32* frac) {
	static char* _suffix[] = {"B", "KB", "MB", "GB", "TB"};
	const char length = sizeof(_suffix) / sizeof(_suffix[0]);
	int i = 0;
	double dblBytes = bytes;

	if (bytes > 1024) {
		for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}
    *suffix = _suffix[i];
    *whole = ((u32) dblBytes);
    *frac = (dblBytes - ((double) *whole)) * 100; 
}
