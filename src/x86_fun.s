.text
        .global _start

_start:
        call   main
        call   _print
        add    $4, %esp
        
        mov    $0x0,%ebx
        mov    $0x1,%eax
        int    $0x80

_print:
         push %ebp
         mov %esp, %ebp
         
         mov $0x1,%ebx
         mov %eax, %ecx
         add $4, %ecx
         mov (%eax), %edx  
         mov $0x4, %eax
         int $0x80
         
         mov %ebp, %esp
         pop %ebp
         ret
            
_malloc:
         push  %ebp
         mov   %esp, %ebp
         
         push  %eax

         # get heap address
         mov   $0x2D, %eax   # sys_brk
         movl  $0, %ebx
         int   $0x80
         
         # new heap address
         mov   -0x4(%ebp), %ebx 
         add   %eax, %ebx
         mov   $0x2D, %eax   # sys_brk
         int   $0x80
         sub   -0x4(%ebp), %eax
       
         add   $4, %esp
         
         mov   %ebp, %esp
         pop   %ebp
         ret

_memcpy:
         push  %ebp
         mov   %esp, %ebp
         
         #mov   0x8(%ebp), %esi
         #mov   0xc(%ebp), %edi
         #mov   0xf(%ebp), %eax
         
         mov   %eax, %esi
         mov   %ebx, %edi
         mov   %ecx, %eax
         
_memcpy_1:
         movsb
         dec   %eax
         cmp   $0, %eax
         jg    _memcpy_1
         
         #mov   0xc(%ebp), %eax
         mov   %ebx, %eax
         
         mov   %ebp, %esp
         pop   %ebp
         ret  

_concat:
         push  %ebp
         mov   %esp, %ebp
         
         push %eax
         push %ebx
         
         # alloc memory
         movl   (%eax), %eax # calculate new length
         addl   (%ebx), %eax
         call  _malloc
         push  %eax
         
         # copy string 1: memcpy(src, dest, size)
         mov   -0x4(%ebp), %ecx
         mov   (%ecx), %ecx
         mov   -0xc(%ebp), %ebx # dest
         add   $4, %ebx
         mov   -0x4(%ebp), %eax # src
         add   $4, %eax
         call  _memcpy
         
         # copy string 2: memcpy(src, dest, size)         
         mov   -0x8(%ebp), %ecx
         mov   (%ecx), %ecx
         mov   -0x4(%ebp), %ebx
         mov   (%ebx), %ebx
         add   -0xc(%ebp), %ebx # dest
         add   $4, %ebx
         mov   -0x8(%ebp), %eax # src
         add   $4, %eax
         call  _memcpy
         
         # final size
         mov   -0x4(%ebp), %eax
         mov   (%eax), %eax
         mov   -0x8(%ebp), %ebx
         mov   (%ebx), %ebx
         mov   -0xc(%ebp), %edx
         add   %ebx, %eax
         mov   %eax, (%edx)
         
         # set result
         mov   -0xc(%ebp), %eax
         
         mov %ebp, %esp
         pop %ebp
         ret
