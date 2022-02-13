

void i386_init()
{
	;
}

void demo()
{
    char *p = 0xa0000;
	for (int i = 0; i <= 0xffff; i++) {
		*(p + i) = 0xf;
	}
}