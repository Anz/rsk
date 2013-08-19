.text
        .global _start

_start:        
        # real program
        pushl  $5
        call   main
        add    $4, %esp
        push   %eax
        call   _print
        add    $4, %esp
        
        mov    $0x0,%ebx
        mov    $0x1,%eax
        int    $0x80

_print:
         push %ebp
         mov %esp, %ebp
         mov 8(%ebp), %ecx
         mov (%ecx), %edx         
         add $4, %ecx
         mov $0x1,%ebx
         mov $0x4, %eax
         int $0x80
         mov %ebp, %esp
         pop %ebp
         ret
            
_malloc:
         push  %ebp
         mov   %esp, %ebp

         # get heap address
         mov   $0x2D, %eax   # sys_brk
         movl  $0, %ebx
         int   $0x80
         
         # new heap address
         mov   0x8(%ebp), %ebx 
         add   %eax, %ebx
         mov   $0x2D, %eax   # sys_brk
         int   $0x80
         sub   0x8(%ebp), %eax
         push %eax
         
         mov   %ebp, %esp
         pop   %ebp
         ret

_memcpy:
         push  %ebp
         mov   %esp, %ebp
         
         mov   0x8(%ebp), %esi
         mov   0xc(%ebp), %edi
         mov   0xf(%ebp), %eax
         
_memcpy_1:
         movsb
         dec   %eax
         cmp   $0, %eax
         jg    _memcpy_1
         
         mov   0xc(%ebp), %eax
         
         mov   %ebp, %esp
         pop   %ebp
         ret  

_concat:
         # init
         push  %ebp
         mov   %esp, %ebp
         
         # calculate new length
         add  $4, %ebx
         mov  8(%ebp), %edx
         mov  (%edx), %edx
         add  %edx, %eax
         mov  0xc(%ebp), %edx
         mov  (%edx), %edx
         add  %edx, %eax
         #push %eax
         
         # alloc memory
         push  %eax
         call  _malloc
         add   $4, %esp
         push  %eax
         
         # copy string 1: memcpy(src, dest, size)
         mov   8(%ebp), %eax
         push  (%eax)
         mov   -0x4(%ebp), %eax # dest
         add   $4, %eax
         push  %eax
         mov   8(%ebp), %eax # src
         add   $4, %eax
         push  %eax
         call  _memcpy
         add   $0xc, %esp
         
         # copy string 2: memcpy(src, dest, size)
         mov   0xc(%ebp), %eax
         push  (%eax)
         mov   8(%ebp), %eax
         mov   (%eax), %eax
         add   -0x4(%ebp), %eax # dest
         add   $4, %eax
         push  %eax
         mov   0xc(%ebp), %eax # src
         add   $4, %eax
         push  %eax
         call  _memcpy
         add   $0xc, %esp
         
         # final size
         mov   8(%ebp), %eax
         mov   (%eax), %eax
         mov   0xc(%ebp), %ebx
         mov   (%ebx), %ebx
         mov   -0x4(%ebp), %edx
         add   %ebx, %eax
         mov   %eax, (%edx)
         
         # print
         mov   -0x4(%ebp), %eax
         
         # out
         mov %ebp, %esp
         pop %ebp
         ret
