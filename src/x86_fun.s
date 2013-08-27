.text
        .global _start

_start:
        call   main
        call   _print
        
        mov    $0x0,%ebx
        mov    $0x1,%eax
        int    $0x80
        
stdin:
         mov   $105, %eax    # 5 bytes
         call  _malloc     # alloc memory
         add   $4, %eax    # skip first 4 byte
         
         mov   %eax, %ecx  # store in string
         mov   $3, %eax    # read syscall
         mov   $0, %ebx    # from stdin
         mov   $100, %edx    # 1 byte
         int   $0x80

         sub   $4, %ecx
         mov   %eax, (%ecx)
         mov   %ecx, %eax
         ret

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
         mov   %eax, %esi
         mov   %ebx, %edi
         mov   %ecx, %eax
         cmp   $0, %eax
         jle    _memcpy_end
_memcpy_loop:
         movsb
         dec   %eax
         cmp   $0, %eax
         jg    _memcpy_loop
         mov   %ebx, %eax
_memcpy_end:     
         mov   %ebp, %esp
         pop   %ebp
         ret

_memcmp:
         mov   %eax, %esi
         mov   %ebx, %edi
         movl  (%esi), %eax
         movl  (%edi), %ebx
         cmp   %eax, %ebx
         jne   _memcmp_false
         mov   %eax, %ecx
         add   $4, %esi
         add   $4, %edi
_memcmp_loop:
         cmp   $1, %ecx
         je    _memcmp_true  
         movb  (%esi), %al
         movb  (%edi), %bl
         cmp   %al, %bl
         jne   _memcmp_false
         inc   %esi
         inc   %edi
         dec   %ecx
         jmp   _memcmp_loop
_memcmp_true:
         mov $0, %eax
         ret
_memcmp_false:
         mov $1, %eax
         ret
         
_concat2:
         push  %ebp
         mov   %esp, %ebp

         push  %ebx
         call  _print
         pop   %eax
         
         mov %ebp, %esp
         pop %ebp
         ret   
         
_concat:
         push  %ebp
         mov   %esp, %ebp
         
         push %eax
         push %ebx
         
         # alloc memory
         addl  (%eax), %eax # calculate new length
         addl  (%ebx), %eax
         addl   $4, %eax
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

_lt:
         sub   %ebx, %eax
         shr   $31, %eax
         add   $-1, %eax
         ret


_gt:
         sub   %eax, %ebx
         shr   $31, %ebx
         mov   %ebx, %eax
         add   $-1, %eax
         ret

_le:
         sub   %eax, %ebx
         shr   $31, %ebx
         mov   %ebx, %eax
         ret


_ge:
         sub   %ebx, %eax
         shr   $31, %eax
         ret

         
