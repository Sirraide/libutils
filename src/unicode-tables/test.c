#include "../../include.h"
#include "../unicode.cc"

int main(void) {
	setlocale(LC_ALL, "");
	FILE *fp = fopen("test.txt", "r");

	while (!feof(fp)) {
		int c = fgetwc(fp);
		if (isstart(c))
			printf(GREEN "%lc" W, c);
		else if (iscont(c))
			printf(YELLOW "%lc" W, c);
		else
			printf("%lc", c);
	}
}
