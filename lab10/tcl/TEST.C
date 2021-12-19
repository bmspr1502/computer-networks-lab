#include <stdio.h>

#define MOBILE 0x01
#define LAPPY 0x02

int main()
{
	unsigned char item = 0x00;

	item |= MOBILE;
	item |= LAPPY;

	printf("I have purchased ...:");
	if (item & MOBILE)
	{
		printf("Mobile, ");
	}
	if (item & LAPPY)
	{
		printf("Lappy");
	}

	return 1;
}
