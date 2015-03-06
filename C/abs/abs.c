#include <stdio.h>

int main(int argc, char *argv[])
{
	int a = 5;
    int b = 0;

    #if 0                   //OK, But this is restricted for param address, for example -4(%ebp) or -8(%ebp)
    asm
    (
        //"mov -4(%ebp), %eax \n\t"                 //vc++6.0 first param is in ebp - 4
        "mov -8(%ebp), %eax \n\t"                   //find first param by assembly that it is in ebp - 8 
        "mov $-1, %edx\n\t"
        "xor %edx, %eax\n\t"
        "sub %edx, %eax\n\t"
        //"mov %eax, -4(%ebp)\n\t"
        "mov %eax, -8(%ebp)\n\t"
    );
    #endif


    #if 0
    /*when using %0,%1...%9, you must add '%%' before registers*/
    /*get -a */
    asm
    (
        "mov %1, %%eax \n\t"                        
        "mov $-1, %%edx\n\t"
        "xor %%edx, %%eax\n\t"
        "sub %%edx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(a) : "m"(a) 
    );
    #endif

    #if 0
    /*if a is positive, get a*/
    asm
    (
        "mov %1, %%eax \n\t"                        
        "mov $0, %%edx\n\t"
        "xor %%edx, %%eax\n\t"
        "sub %%edx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(a) : "m"(a) 
    );
    #endif
    
    /*get the absolute value of a*/
    asm
    (
        "mov %1, %%eax \n\t"   
        "mov %%eax, %%ecx \n\t" 
        "mov $-1, %%edx\n\t"
        "shr $31, %%ecx\n\t" 
        "jnz LP\n\t"                     
        "mov $0, %%edx\n\t"
        "LP:\n\t"
        "xor %%edx, %%eax\n\t"
        "sub %%edx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(a) : "m"(a) 
    );

    /*
    -->
    int temp = a;
    temp = temp >> 31;
    a = a ^ temp;
    a = a - temp;
    */ 
    
    

    printf("a:%p\n", &a);
    printf("b:%p\n", &b);
    printf("%d\n", a);

	return 0;
}
