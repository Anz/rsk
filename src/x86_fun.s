.text
        .global _start

_start:
        call _concat
        push $5
        call main
        sub $0, %esp
        mov $0x0,%ebx
        mov $0x1,%eax
        int $0x80

_concat:
         # get heap address
         mov $0x2D, %eax   # sys_brk
         movl $0, %ebx
         int $0x80
         
         # new heap address
         add $4, %eax
         add (%esp), %eax
         
         # alloc memory
         mov %eax, %ebx
         mov $0x2D, %eax   # sys_brk
         int $0x80
         ret
