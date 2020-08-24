#include <stdio.h>
#include "smalloc.h"

int 
main()
{
	void *p1, *p2, *p3, *p4, *p5 ;

	print_sm_containers() ;

	p1 = smalloc(2000) ; 
	printf("smalloc(2000):%p\n", p1) ; 
	print_sm_containers() ;
    print_mem_uses();

	p2 = smalloc(2500) ; 
	printf("smalloc(2500):%p\n", p2) ; 
	print_sm_containers() ;
    print_mem_uses();

	p3 = smalloc(1000) ; 
	printf("smalloc(1000):%p\n", p3) ; 
	print_sm_containers() ;
    print_mem_uses();

	sfree(p2) ; 
	printf("sfree(%p)\n", p2); 
	print_sm_containers() ;
    print_mem_uses();

	p3 = srealloc(p3, 1500) ; 
	printf("srealloc(p3, 1500):%p\n", p3) ; 
	print_sm_containers() ;
    print_mem_uses();

	sshrink(); 
	printf("sshrink()\n") ; 
	print_sm_containers() ;
    print_mem_uses();

	p4 = smalloc(1000) ; 
	printf("smalloc(1000):%p\n", p4) ; 
	print_sm_containers() ;
    print_mem_uses();

	p5 = smalloc(5000) ; 
	printf("smalloc(5000):%p\n", p5) ; 
	print_sm_containers() ;
    print_mem_uses();

	sfree(p5) ; 
	printf("sfree(%p)\n", p5); 
	print_sm_containers() ;
    print_mem_uses();

    sshrink(); 
	printf("sshrink()\n") ; 
	print_sm_containers() ;
    print_mem_uses();
}