.section .text
.globl  main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $128, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_0(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    subq    $16, %rsp
    leaq    .Lxly_str_1(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    subq    $16, %rsp
    leaq    .Lxly_str_2(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4629137466983448576, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_validateAge
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rdi
    call    xly_neg
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_validateAge
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4641240890982006784, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_validateAge
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    subq    $16, %rsp
    leaq    .Lxly_str_3(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_double
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_double
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    call    xly_null
    pushq   %rax
    call    xly_null
    pushq   %rax
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_compose
    movq    %rax, -8(%rbp)
    subq    $16, %rsp
    leaq    .Lxly_str_4(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    subq    $16, %rsp
    leaq    .Lxly_str_5(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_6(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_8(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_9(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movq    %rsp, %rdi
    movl    $0, %esi
    call    xly_print
    subq    $16, %rsp
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdi
    movl    $1, %esi
    call    xly_print
    addq    $16, %rsp
    movl    $0, %edi
    call    xly_exit
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_validateAge:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $80, %rsp
    movq    %rdi, -8(%rbp)
    movq    -8(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_2
    cmpl    $0, (%rsi)
    jne     .Lxly_2
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_3
.Lxly_2:
    call    xly_lt
.Lxly_3:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_0
    movl    $0, %edi
    call    xly_bool
    movq    %rbp, %rsp
    popq    %rbp
    ret
    jmp     .Lxly_1
.Lxly_0:
.Lxly_1:
    movq    -8(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4639481672377565184, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_6
    cmpl    $0, (%rsi)
    jne     .Lxly_6
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    seta    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_7
.Lxly_6:
    call    xly_gt
.Lxly_7:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_4
    movl    $0, %edi
    call    xly_bool
    movq    %rbp, %rsp
    popq    %rbp
    ret
    jmp     .Lxly_5
.Lxly_4:
.Lxly_5:
    movl    $1, %edi
    call    xly_bool
    movq    %rbp, %rsp
    popq    %rbp
    ret
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_double:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $80, %rsp
    movq    %rdi, -8(%rbp)
    movq    -8(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4611686018427387904, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    mulsd   %xmm1, %xmm0
    call    xly_num
    movq    %rbp, %rsp
    popq    %rbp
    ret
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_compose:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    movq    -24(%rbp), %rax
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_g
    pushq   %rax
    popq    %rdi
    call    .Lxly_fn_f
    movq    %rbp, %rsp
    popq    %rbp
    ret
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_addOne:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $80, %rsp
    movq    %rdi, -8(%rbp)
    movq    -8(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_8
    cmpl    $1, (%rsi)
    je      .Lxly_8
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_9
.Lxly_8:
    call    xly_add
.Lxly_9:
    movq    %rbp, %rsp
    popq    %rbp
    ret
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.section .rodata
.Lxly_str_0:
    .asciz  "Declarative Programming"
.Lxly_str_1:
    .asciz  "Focus on WHAT, not HOW"
.Lxly_str_2:
    .asciz  "Validation:"
.Lxly_str_3:
    .asciz  "Transformation:"
.Lxly_str_4:
    .asciz  "Composed:"
.Lxly_str_5:
    .asciz  "Benefits:"
.Lxly_str_6:
    .asciz  "  Clearer intent"
.Lxly_str_7:
    .asciz  "  Less code"
.Lxly_str_8:
    .asciz  "  Easier to read"
.Lxly_str_9:
    .asciz  "  More maintainable"
.Lxly_str_10:
    .asciz  "Complete!"

.section .note.GNU-stack,"",@progbits
