.section .text
.globl  main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $608, %rsp
    leaq    .Lxly_str_0(%rip), %rdi
    call    xly_str
    movq    %rax, -8(%rbp)
    leaq    .Lxly_str_1(%rip), %rdi
    call    xly_str
    movq    %rax, -16(%rbp)
    xorq    %rdx, %rdx
    leaq    .Lxly_str_2(%rip), %rdi
    leaq    .Lxly_str_3(%rip), %rsi
    movl    $0, %ecx
    call    xly_call_module
    movq    %rax, -24(%rbp)
    subq    $8, %rsp
    movabsq $4634415122796773376, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -32(%rbp)
    subq    $8, %rsp
    movabsq $4662659377491083264, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -40(%rbp)
    subq    $8, %rsp
    movabsq $4670232813583204352, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -48(%rbp)
    subq    $8, %rsp
    movabsq $4644337115725824000, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -56(%rbp)
    subq    $8, %rsp
    movabsq $4647714815446351872, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -64(%rbp)
    subq    $8, %rsp
    movabsq $4638426141214900224, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -72(%rbp)
    subq    $8, %rsp
    movabsq $4630544841867001856, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -80(%rbp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -88(%rbp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -96(%rbp)
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -104(%rbp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -112(%rbp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -120(%rbp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -128(%rbp)
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -136(%rbp)
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4625196817309499392, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_5(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -144(%rbp)
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4627167142146473984, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4629418941960159232, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4611686018427387904, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4631530004285489152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4630544841867001856, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4616189618054758400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4632515166703976448, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4633781804099174400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4618441417868443648, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4632937379169042432, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4631952216750555136, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4620693217682128896, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634626229029306368, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4621256167635550208, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4635470653959438336, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634978072750194688, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4622382067542392832, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634555860285128704, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4622945017495814144, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634837335261839360, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4623507967449235456, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634415122796773376, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4624070917402656768, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634696597773484032, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -144(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4624633867356078080, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    subq    $8, %rsp
    movabsq $4634415122796773376, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    movq    -8(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_6
    cmpl    $1, (%rsi)
    je      .Lxly_6
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_7
.Lxly_6:
    call    xly_add
.Lxly_7:
    pushq   %rax
    movq    -16(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_4
    cmpl    $1, (%rsi)
    je      .Lxly_4
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_5
.Lxly_4:
    call    xly_add
.Lxly_5:
    pushq   %rax
    leaq    .Lxly_str_11(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_2
    cmpl    $1, (%rsi)
    je      .Lxly_2
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_3
.Lxly_2:
    call    xly_add
.Lxly_3:
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_0
    cmpl    $1, (%rsi)
    je      .Lxly_0
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_1
.Lxly_0:
    call    xly_add
.Lxly_1:
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawStatusBar
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4630826316843712512, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_12(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4616189618054758400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_13(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -32(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_10
    cmpl    $1, (%rsi)
    je      .Lxly_10
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_11
.Lxly_10:
    call    xly_add
.Lxly_11:
    pushq   %rax
    leaq    .Lxly_str_14(%rip), %rdi
    call    xly_str
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
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawLabel
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    movq    -32(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4628574517030027264, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawProgressBar
    subq    $8, %rsp
    movabsq $4618441417868443648, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_15(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -40(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_18
    cmpl    $1, (%rsi)
    je      .Lxly_18
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_19
.Lxly_18:
    call    xly_add
.Lxly_19:
    pushq   %rax
    leaq    .Lxly_str_16(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_16
    cmpl    $1, (%rsi)
    je      .Lxly_16
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_17
.Lxly_16:
    call    xly_add
.Lxly_17:
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_14
    cmpl    $1, (%rsi)
    je      .Lxly_14
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_15
.Lxly_14:
    call    xly_add
.Lxly_15:
    pushq   %rax
    leaq    .Lxly_str_17(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_12
    cmpl    $1, (%rsi)
    je      .Lxly_12
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_13
.Lxly_12:
    call    xly_add
.Lxly_13:
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawLabel
    movq    -40(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
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
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -160(%rbp)
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    movq    -160(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4628574517030027264, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawProgressBar
    subq    $8, %rsp
    movabsq $4620693217682128896, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_18(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -56(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_26
    cmpl    $1, (%rsi)
    je      .Lxly_26
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_27
.Lxly_26:
    call    xly_add
.Lxly_27:
    pushq   %rax
    leaq    .Lxly_str_16(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_24
    cmpl    $1, (%rsi)
    je      .Lxly_24
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_25
.Lxly_24:
    call    xly_add
.Lxly_25:
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_22
    cmpl    $1, (%rsi)
    je      .Lxly_22
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_23
.Lxly_22:
    call    xly_add
.Lxly_23:
    pushq   %rax
    leaq    .Lxly_str_19(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_20
    cmpl    $1, (%rsi)
    je      .Lxly_20
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_21
.Lxly_20:
    call    xly_add
.Lxly_21:
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawLabel
    movq    -56(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
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
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -168(%rbp)
    subq    $8, %rsp
    movabsq $4621256167635550208, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    movq    -168(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4628574517030027264, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawProgressBar
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_20(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -72(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_30
    cmpl    $1, (%rsi)
    je      .Lxly_30
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_31
.Lxly_30:
    call    xly_add
.Lxly_31:
    pushq   %rax
    leaq    .Lxly_str_21(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_28
    cmpl    $1, (%rsi)
    je      .Lxly_28
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_29
.Lxly_28:
    call    xly_add
.Lxly_29:
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawLabel
    subq    $8, %rsp
    movabsq $4622382067542392832, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_22(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -80(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_34
    cmpl    $1, (%rsi)
    je      .Lxly_34
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_35
.Lxly_34:
    call    xly_add
.Lxly_35:
    pushq   %rax
    leaq    .Lxly_str_21(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_32
    cmpl    $1, (%rsi)
    je      .Lxly_32
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_33
.Lxly_32:
    call    xly_add
.Lxly_33:
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawLabel
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631107791820423168, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4630544841867001856, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_23(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4616189618054758400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $16, %rsp
    leaq    .Lxly_str_24(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    movq    -144(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawSparkline
    subq    $8, %rsp
    movabsq $4622382067542392832, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $16, %rsp
    leaq    .Lxly_str_25(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiBold
    call    .Lxly_fn_xuiFgBrightYellow
    subq    $16, %rsp
    leaq    .Lxly_str_26(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4618441417868443648, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiBold
    call    .Lxly_fn_xuiFgBrightGreen
    subq    $16, %rsp
    leaq    .Lxly_str_27(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -32(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_38
    cmpl    $1, (%rsi)
    je      .Lxly_38
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_39
.Lxly_38:
    call    xly_add
.Lxly_39:
    pushq   %rax
    leaq    .Lxly_str_14(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_36
    cmpl    $1, (%rsi)
    je      .Lxly_36
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_37
.Lxly_36:
    call    xly_add
.Lxly_37:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4624070917402656768, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4630826316843712512, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_28(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4624633867356078080, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_29(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -88(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625196817309499392, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_30(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -96(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625478292286210048, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_31(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -104(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625759767262920704, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_32(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -112(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    movq    -88(%rbp), %rax
    pushq   %rax
    movq    -96(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_44
    cmpl    $1, (%rsi)
    je      .Lxly_44
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_45
.Lxly_44:
    call    xly_add
.Lxly_45:
    pushq   %rax
    movq    -104(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_42
    cmpl    $1, (%rsi)
    je      .Lxly_42
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_43
.Lxly_42:
    call    xly_add
.Lxly_43:
    pushq   %rax
    movq    -112(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_40
    cmpl    $1, (%rsi)
    je      .Lxly_40
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_41
.Lxly_40:
    call    xly_add
.Lxly_41:
    movq    %rax, -176(%rbp)
    subq    $8, %rsp
    movabsq $4626041242239631360, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    leaq    .Lxly_str_33(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4626041242239631360, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4622945017495814144, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -176(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4616189618054758400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_48
    cmpl    $0, (%rsi)
    jne     .Lxly_48
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_49
.Lxly_48:
    call    xly_eq
.Lxly_49:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_46
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -176(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_52
    cmpl    $1, (%rsi)
    je      .Lxly_52
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_53
.Lxly_52:
    call    xly_add
.Lxly_53:
    pushq   %rax
    leaq    .Lxly_str_34(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_50
    cmpl    $1, (%rsi)
    je      .Lxly_50
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_51
.Lxly_50:
    call    xly_add
.Lxly_51:
    pushq   %rax
    leaq    .Lxly_str_35(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawBadge
    jmp     .Lxly_47
.Lxly_46:
.Lxly_47:
    movq    -176(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_56
    cmpl    $0, (%rsi)
    jne     .Lxly_56
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_57
.Lxly_56:
    call    xly_eq
.Lxly_57:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_54
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -176(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_60
    cmpl    $1, (%rsi)
    je      .Lxly_60
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_61
.Lxly_60:
    call    xly_add
.Lxly_61:
    pushq   %rax
    leaq    .Lxly_str_34(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_58
    cmpl    $1, (%rsi)
    je      .Lxly_58
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_59
.Lxly_58:
    call    xly_add
.Lxly_59:
    pushq   %rax
    leaq    .Lxly_str_36(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawBadge
    jmp     .Lxly_55
.Lxly_54:
.Lxly_55:
    movq    -176(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_64
    cmpl    $0, (%rsi)
    jne     .Lxly_64
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_65
.Lxly_64:
    call    xly_lt
.Lxly_65:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_62
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -176(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_68
    cmpl    $1, (%rsi)
    je      .Lxly_68
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_69
.Lxly_68:
    call    xly_add
.Lxly_69:
    pushq   %rax
    leaq    .Lxly_str_34(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_66
    cmpl    $1, (%rsi)
    je      .Lxly_66
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_67
.Lxly_66:
    call    xly_add
.Lxly_67:
    pushq   %rax
    leaq    .Lxly_str_37(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawBadge
    jmp     .Lxly_63
.Lxly_62:
.Lxly_63:
    subq    $8, %rsp
    movabsq $4624070917402656768, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631107791820423168, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4630544841867001856, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_38(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4624633867356078080, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_39(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -120(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625196817309499392, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_40(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -128(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625478292286210048, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_41(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -136(%rbp), %rax
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawCheckbox
    subq    $8, %rsp
    movabsq $4625759767262920704, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4631389266797133824, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $16, %rsp
    leaq    .Lxly_str_42(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    .Lxly_fn_xuiFgBrightCyan
    subq    $16, %rsp
    movq    -24(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4626885667169763328, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4621819117588971520, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4635259547726905344, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_43(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4625196817309499392, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -184(%rbp)
    subq    $8, %rsp
    movabsq $4627167142146473984, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_44(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_45(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_46(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_47(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableHeader
    subq    $8, %rsp
    movabsq $4627448617123184640, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $16, %rsp
    leaq    .Lxly_str_48(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4627730092099895296, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_49(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_50(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_51(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_52(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableRow
    subq    $8, %rsp
    movabsq $4628011567076605952, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_53(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_54(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_55(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_56(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableRow
    subq    $8, %rsp
    movabsq $4628293042053316608, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_57(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_58(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_59(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_60(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableRow
    subq    $8, %rsp
    movabsq $4628574517030027264, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_61(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_62(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_63(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_64(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableRow
    subq    $8, %rsp
    movabsq $4628855992006737920, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_65(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_66(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_67(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_68(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -184(%rbp), %rax
    pushq   %rax
    popq    %r9
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawTableRow
    subq    $8, %rsp
    movabsq $4629841154425225216, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4618441417868443648, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4635259547726905344, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    leaq    .Lxly_str_40(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %r8
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawPanel
    subq    $8, %rsp
    movabsq $4629981891913580544, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_69(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_35(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawNotify
    subq    $8, %rsp
    movabsq $4630122629401935872, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_70(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_36(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawNotify
    subq    $8, %rsp
    movabsq $4630263366890291200, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_71(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_72(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawNotify
    subq    $8, %rsp
    movabsq $4630404104378646528, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_73(%rip), %rdi
    call    xly_str
    pushq   %rax
    leaq    .Lxly_str_72(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rdx
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawNotify
    subq    $8, %rsp
    movabsq $4630826316843712512, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    movq    -152(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_74(%rip), %rdi
    call    xly_str
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_drawStatusBar
    subq    $8, %rsp
    movabsq $4631107791820423168, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -152(%rbp)
    subq    $16, %rsp
    movq    -152(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $16, %rsp
    leaq    .Lxly_str_75(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movl    $0, %edi
    call    xly_exit
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiPos:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -8(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_70
    cmpl    $1, (%rsi)
    je      .Lxly_70
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_71
.Lxly_70:
    call    xly_add
.Lxly_71:
    movq    %rax, -24(%rbp)
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -16(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_72
    cmpl    $1, (%rsi)
    je      .Lxly_72
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_73
.Lxly_72:
    call    xly_add
.Lxly_73:
    movq    %rax, -32(%rbp)
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_80
    cmpl    $1, (%rsi)
    je      .Lxly_80
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_81
.Lxly_80:
    call    xly_add
.Lxly_81:
    pushq   %rax
    leaq    .Lxly_str_76(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_78
    cmpl    $1, (%rsi)
    je      .Lxly_78
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_79
.Lxly_78:
    call    xly_add
.Lxly_79:
    pushq   %rax
    movq    -32(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_76
    cmpl    $1, (%rsi)
    je      .Lxly_76
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_77
.Lxly_76:
    call    xly_add
.Lxly_77:
    pushq   %rax
    leaq    .Lxly_str_77(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_74
    cmpl    $1, (%rsi)
    je      .Lxly_74
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_75
.Lxly_74:
    call    xly_add
.Lxly_75:
    movq    %rbp, %rsp
    popq    %rbp
    ret
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiReset:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBold:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiDim:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBlack:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgGreen:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgYellow:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBlue:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgCyan:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgWhite:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightBlack:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightRed:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightGreen:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightYellow:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightBlue:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightCyan:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiFgBrightWhite:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgBlack:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgRed:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgGreen:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgBlue:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgYellow:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgCyan:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgBrightBlack:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_xuiBgBrightBlue:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawPanel:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $176, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    movq    %rcx, -32(%rbp)
    movq    %r8, -40(%rbp)
    movq    -32(%rbp), %rax
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
    subsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -48(%rbp)
    subq    $16, %rsp
    movq    -40(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -56(%rbp)
    movq    -48(%rbp), %rax
    pushq   %rax
    movq    -56(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    subsd   %xmm1, %xmm0
    call    xly_num
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
    subsd   %xmm1, %xmm0
    call    xly_num
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
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -64(%rbp)
    movq    -8(%rbp), %rax
    pushq   %rax
    movq    -16(%rbp), %rax
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -72(%rbp)
    subq    $16, %rsp
    movq    -72(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgCyan
    subq    $16, %rsp
    leaq    .Lxly_str_80(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -80(%rbp)
.Lxly_82:
    movq    -80(%rbp), %rax
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_84
    cmpl    $0, (%rsi)
    jne     .Lxly_84
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_85
.Lxly_84:
    call    xly_lt
.Lxly_85:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_83
    subq    $16, %rsp
    leaq    .Lxly_str_81(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -80(%rbp), %rax
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
    je      .Lxly_86
    cmpl    $1, (%rsi)
    je      .Lxly_86
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_87
.Lxly_86:
    call    xly_add
.Lxly_87:
    movq    %rax, -80(%rbp)
    jmp     .Lxly_82
.Lxly_83:
    subq    $16, %rsp
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiBold
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    movq    -40(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    .Lxly_fn_xuiFgCyan
    subq    $16, %rsp
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -80(%rbp)
.Lxly_88:
    movq    -80(%rbp), %rax
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_90
    cmpl    $0, (%rsi)
    jne     .Lxly_90
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_91
.Lxly_90:
    call    xly_lt
.Lxly_91:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_89
    subq    $16, %rsp
    leaq    .Lxly_str_81(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -80(%rbp), %rax
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
    je      .Lxly_92
    cmpl    $1, (%rsi)
    je      .Lxly_92
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_93
.Lxly_92:
    call    xly_add
.Lxly_93:
    movq    %rax, -80(%rbp)
    jmp     .Lxly_88
.Lxly_89:
    subq    $16, %rsp
    leaq    .Lxly_str_83(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -80(%rbp)
.Lxly_94:
    movq    -80(%rbp), %rax
    pushq   %rax
    movq    -24(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    subsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_96
    cmpl    $0, (%rsi)
    jne     .Lxly_96
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_97
.Lxly_96:
    call    xly_lt
.Lxly_97:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_95
    movq    -8(%rbp), %rax
    pushq   %rax
    movq    -80(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_98
    cmpl    $1, (%rsi)
    je      .Lxly_98
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_99
.Lxly_98:
    call    xly_add
.Lxly_99:
    movq    %rax, -88(%rbp)
    movq    -88(%rbp), %rax
    pushq   %rax
    movq    -16(%rbp), %rax
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -72(%rbp)
    subq    $16, %rsp
    movq    -72(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgCyan
    subq    $16, %rsp
    leaq    .Lxly_str_84(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    movq    -16(%rbp), %rax
    pushq   %rax
    movq    -32(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_100
    cmpl    $1, (%rsi)
    je      .Lxly_100
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_101
.Lxly_100:
    call    xly_add
.Lxly_101:
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    subsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -96(%rbp)
    movq    -88(%rbp), %rax
    pushq   %rax
    movq    -96(%rbp), %rax
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -72(%rbp)
    subq    $16, %rsp
    movq    -72(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgCyan
    subq    $16, %rsp
    leaq    .Lxly_str_84(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    movq    -80(%rbp), %rax
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
    je      .Lxly_102
    cmpl    $1, (%rsi)
    je      .Lxly_102
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_103
.Lxly_102:
    call    xly_add
.Lxly_103:
    movq    %rax, -80(%rbp)
    jmp     .Lxly_94
.Lxly_95:
    movq    -8(%rbp), %rax
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_104
    cmpl    $1, (%rsi)
    je      .Lxly_104
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_105
.Lxly_104:
    call    xly_add
.Lxly_105:
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    subsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -104(%rbp)
    movq    -104(%rbp), %rax
    pushq   %rax
    movq    -16(%rbp), %rax
    pushq   %rax
    popq    %rsi
    popq    %rdi
    call    .Lxly_fn_xuiPos
    movq    %rax, -72(%rbp)
    subq    $16, %rsp
    movq    -72(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgCyan
    subq    $16, %rsp
    leaq    .Lxly_str_85(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -80(%rbp)
.Lxly_106:
    movq    -80(%rbp), %rax
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_108
    cmpl    $0, (%rsi)
    jne     .Lxly_108
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_109
.Lxly_108:
    call    xly_lt
.Lxly_109:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_107
    subq    $16, %rsp
    leaq    .Lxly_str_81(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -80(%rbp), %rax
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
    je      .Lxly_110
    cmpl    $1, (%rsi)
    je      .Lxly_110
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_111
.Lxly_110:
    call    xly_add
.Lxly_111:
    movq    %rax, -80(%rbp)
    jmp     .Lxly_106
.Lxly_107:
    subq    $16, %rsp
    leaq    .Lxly_str_86(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawProgressBar:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $128, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    movq    %rcx, -32(%rbp)
    movq    -16(%rbp), %rax
    pushq   %rax
    movq    -32(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    mulsd   %xmm1, %xmm0
    call    xly_num
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -40(%rbp)
    movq    -32(%rbp), %rax
    pushq   %rax
    movq    -40(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    subsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -48(%rbp)
    movq    -16(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4636737291354636288, %rax
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
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -56(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    leaq    .Lxly_str_87(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgBrightGreen
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -64(%rbp)
.Lxly_112:
    movq    -64(%rbp), %rax
    pushq   %rax
    movq    -40(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_114
    cmpl    $0, (%rsi)
    jne     .Lxly_114
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_115
.Lxly_114:
    call    xly_lt
.Lxly_115:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_113
    subq    $16, %rsp
    leaq    .Lxly_str_88(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -64(%rbp), %rax
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
    je      .Lxly_116
    cmpl    $1, (%rsi)
    je      .Lxly_116
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_117
.Lxly_116:
    call    xly_add
.Lxly_117:
    movq    %rax, -64(%rbp)
    jmp     .Lxly_112
.Lxly_113:
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -64(%rbp)
.Lxly_118:
    movq    -64(%rbp), %rax
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_120
    cmpl    $0, (%rsi)
    jne     .Lxly_120
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_121
.Lxly_120:
    call    xly_lt
.Lxly_121:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_119
    subq    $16, %rsp
    leaq    .Lxly_str_89(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -64(%rbp), %rax
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
    je      .Lxly_122
    cmpl    $1, (%rsi)
    je      .Lxly_122
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_123
.Lxly_122:
    call    xly_add
.Lxly_123:
    movq    %rax, -64(%rbp)
    jmp     .Lxly_118
.Lxly_119:
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    leaq    .Lxly_str_90(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgBrightYellow
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_7(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -56(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_126
    cmpl    $1, (%rsi)
    je      .Lxly_126
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_127
.Lxly_126:
    call    xly_add
.Lxly_127:
    pushq   %rax
    leaq    .Lxly_str_14(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_124
    cmpl    $1, (%rsi)
    je      .Lxly_124
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_125
.Lxly_124:
    call    xly_add
.Lxly_125:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawBadge:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_35(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_130
    cmpl    $0, (%rsi)
    jne     .Lxly_130
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_131
.Lxly_130:
    call    xly_eq
.Lxly_131:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_128
    call    .Lxly_fn_xuiBgGreen
    call    .Lxly_fn_xuiFgBlack
    call    .Lxly_fn_xuiBold
    jmp     .Lxly_129
.Lxly_128:
.Lxly_129:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_37(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_134
    cmpl    $0, (%rsi)
    jne     .Lxly_134
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_135
.Lxly_134:
    call    xly_eq
.Lxly_135:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_132
    call    .Lxly_fn_xuiBgRed
    call    .Lxly_fn_xuiFgBrightWhite
    call    .Lxly_fn_xuiBold
    jmp     .Lxly_133
.Lxly_132:
.Lxly_133:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_36(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_138
    cmpl    $0, (%rsi)
    jne     .Lxly_138
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_139
.Lxly_138:
    call    xly_eq
.Lxly_139:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_136
    call    .Lxly_fn_xuiBgYellow
    call    .Lxly_fn_xuiFgBlack
    call    .Lxly_fn_xuiBold
    jmp     .Lxly_137
.Lxly_136:
.Lxly_137:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_72(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_142
    cmpl    $0, (%rsi)
    jne     .Lxly_142
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_143
.Lxly_142:
    call    xly_eq
.Lxly_143:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_140
    call    .Lxly_fn_xuiBgBlue
    call    .Lxly_fn_xuiFgBrightWhite
    call    .Lxly_fn_xuiBold
    jmp     .Lxly_141
.Lxly_140:
.Lxly_141:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_91(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_146
    cmpl    $0, (%rsi)
    jne     .Lxly_146
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_147
.Lxly_146:
    call    xly_eq
.Lxly_147:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_144
    call    .Lxly_fn_xuiBgBrightBlack
    call    .Lxly_fn_xuiFgWhite
    jmp     .Lxly_145
.Lxly_144:
.Lxly_145:
    subq    $16, %rsp
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -16(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_150
    cmpl    $1, (%rsi)
    je      .Lxly_150
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_151
.Lxly_150:
    call    xly_add
.Lxly_151:
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_148
    cmpl    $1, (%rsi)
    je      .Lxly_148
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_149
.Lxly_148:
    call    xly_add
.Lxly_149:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawSparkline:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $128, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    subq    $16, %rsp
    subq    $8, %rsp
    movabsq $4620693217682128896, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_5(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -32(%rbp)
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_92(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_93(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4611686018427387904, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_94(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4613937818241073152, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_95(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4616189618054758400, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_96(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4617315517961601024, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_97(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4618441417868443648, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_98(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $32, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, 8(%rsp)
    leaq    .Lxly_str_88(%rip), %rdi
    call    xly_str
    movq    %rax, 16(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_6(%rip), %rsi
    movl    $3, %ecx
    call    xly_call_module
    addq    $32, %rsp
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiFgBrightGreen
    subq    $8, %rsp
    movabsq $0, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, -40(%rbp)
.Lxly_152:
    movq    -40(%rbp), %rax
    pushq   %rax
    subq    $16, %rsp
    movq    -16(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_154
    cmpl    $0, (%rsi)
    jne     .Lxly_154
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_155
.Lxly_154:
    call    xly_lt
.Lxly_155:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_153
    subq    $16, %rsp
    movq    -16(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    -40(%rbp), %rax
    movq    %rax, 8(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_99(%rip), %rsi
    movl    $2, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -48(%rbp)
    movq    -48(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4619567317775286272, %rax
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
    pushq   %rax
    movq    -24(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    divsd   %xmm1, %xmm0
    call    xly_num
    movq    %rax, -56(%rbp)
    subq    $16, %rsp
    movq    -32(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    -56(%rbp), %rax
    movq    %rax, 8(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_4(%rip), %rdi
    leaq    .Lxly_str_99(%rip), %rsi
    movl    $2, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -64(%rbp)
    subq    $16, %rsp
    movq    -64(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -40(%rbp), %rax
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
    je      .Lxly_156
    cmpl    $1, (%rsi)
    je      .Lxly_156
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_157
.Lxly_156:
    call    xly_add
.Lxly_157:
    movq    %rax, -40(%rbp)
    jmp     .Lxly_152
.Lxly_153:
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawStatusBar:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiBgBrightBlue
    call    .Lxly_fn_xuiFgBrightWhite
    call    .Lxly_fn_xuiBold
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -16(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_158
    cmpl    $1, (%rsi)
    je      .Lxly_158
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_159
.Lxly_158:
    call    xly_add
.Lxly_159:
    movq    %rax, -24(%rbp)
    subq    $16, %rsp
    movq    -24(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -32(%rbp)
.Lxly_160:
    movq    -32(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4635329916471083008, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_162
    cmpl    $0, (%rsi)
    jne     .Lxly_162
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_163
.Lxly_162:
    call    xly_lt
.Lxly_163:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_161
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_164
    cmpl    $1, (%rsi)
    je      .Lxly_164
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_165
.Lxly_164:
    call    xly_add
.Lxly_165:
    movq    %rax, -24(%rbp)
    movq    -32(%rbp), %rax
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
    je      .Lxly_166
    cmpl    $1, (%rsi)
    je      .Lxly_166
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_167
.Lxly_166:
    call    xly_add
.Lxly_167:
    movq    %rax, -32(%rbp)
    jmp     .Lxly_160
.Lxly_161:
    subq    $16, %rsp
    movq    -24(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawCheckbox:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -24(%rbp), %rax
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_170
    cmpl    $0, (%rsi)
    jne     .Lxly_170
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_171
.Lxly_170:
    call    xly_eq
.Lxly_171:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_168
    call    .Lxly_fn_xuiFgBrightGreen
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_100(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_169
.Lxly_168:
.Lxly_169:
    movq    -24(%rbp), %rax
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
    jne     .Lxly_174
    cmpl    $0, (%rsi)
    jne     .Lxly_174
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_175
.Lxly_174:
    call    xly_eq
.Lxly_175:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_172
    call    .Lxly_fn_xuiFgBrightBlack
    subq    $16, %rsp
    leaq    .Lxly_str_101(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_173
.Lxly_172:
.Lxly_173:
    call    .Lxly_fn_xuiReset
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    movq    -16(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawLabel:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiBold
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    movq    -16(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_102(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_176
    cmpl    $1, (%rsi)
    je      .Lxly_176
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_177
.Lxly_176:
    call    xly_add
.Lxly_177:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    .Lxly_fn_xuiFgBrightYellow
    subq    $16, %rsp
    movq    -24(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawNotify:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $96, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_35(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_180
    cmpl    $0, (%rsi)
    jne     .Lxly_180
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_181
.Lxly_180:
    call    xly_eq
.Lxly_181:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_178
    call    .Lxly_fn_xuiFgBrightGreen
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_103(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_179
.Lxly_178:
.Lxly_179:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_37(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_184
    cmpl    $0, (%rsi)
    jne     .Lxly_184
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_185
.Lxly_184:
    call    xly_eq
.Lxly_185:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_182
    call    .Lxly_fn_xuiFgBrightRed
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_104(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_183
.Lxly_182:
.Lxly_183:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_72(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_188
    cmpl    $0, (%rsi)
    jne     .Lxly_188
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_189
.Lxly_188:
    call    xly_eq
.Lxly_189:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_186
    call    .Lxly_fn_xuiFgBrightCyan
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_105(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_187
.Lxly_186:
.Lxly_187:
    movq    -24(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_36(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_192
    cmpl    $0, (%rsi)
    jne     .Lxly_192
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_193
.Lxly_192:
    call    xly_eq
.Lxly_193:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_190
    call    .Lxly_fn_xuiFgBrightYellow
    call    .Lxly_fn_xuiBold
    subq    $16, %rsp
    leaq    .Lxly_str_106(%rip), %rdi
    call    xly_str
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    jmp     .Lxly_191
.Lxly_190:
.Lxly_191:
    call    .Lxly_fn_xuiReset
    call    .Lxly_fn_xuiFgBrightWhite
    subq    $16, %rsp
    movq    -16(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawTableHeader:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $176, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    movq    %rcx, -32(%rbp)
    movq    %r8, -40(%rbp)
    movq    %r9, -184(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiBold
    call    .Lxly_fn_xuiBgBrightBlack
    call    .Lxly_fn_xuiFgBrightCyan
    movq    -16(%rbp), %rax
    movq    %rax, -48(%rbp)
    subq    $16, %rsp
    movq    -48(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -56(%rbp)
.Lxly_194:
    movq    -56(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_196
    cmpl    $0, (%rsi)
    jne     .Lxly_196
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_197
.Lxly_196:
    call    xly_lt
.Lxly_197:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_195
    movq    -48(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_198
    cmpl    $1, (%rsi)
    je      .Lxly_198
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_199
.Lxly_198:
    call    xly_add
.Lxly_199:
    movq    %rax, -48(%rbp)
    movq    -56(%rbp), %rax
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
    je      .Lxly_200
    cmpl    $1, (%rsi)
    je      .Lxly_200
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_201
.Lxly_200:
    call    xly_add
.Lxly_201:
    movq    %rax, -56(%rbp)
    jmp     .Lxly_194
.Lxly_195:
    movq    -24(%rbp), %rax
    movq    %rax, -64(%rbp)
    subq    $16, %rsp
    movq    -64(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -72(%rbp)
.Lxly_202:
    movq    -72(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_204
    cmpl    $0, (%rsi)
    jne     .Lxly_204
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_205
.Lxly_204:
    call    xly_lt
.Lxly_205:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_203
    movq    -64(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_206
    cmpl    $1, (%rsi)
    je      .Lxly_206
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_207
.Lxly_206:
    call    xly_add
.Lxly_207:
    movq    %rax, -64(%rbp)
    movq    -72(%rbp), %rax
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
    je      .Lxly_208
    cmpl    $1, (%rsi)
    je      .Lxly_208
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_209
.Lxly_208:
    call    xly_add
.Lxly_209:
    movq    %rax, -72(%rbp)
    jmp     .Lxly_202
.Lxly_203:
    movq    -32(%rbp), %rax
    movq    %rax, -80(%rbp)
    subq    $16, %rsp
    movq    -80(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -88(%rbp)
.Lxly_210:
    movq    -88(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_212
    cmpl    $0, (%rsi)
    jne     .Lxly_212
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_213
.Lxly_212:
    call    xly_lt
.Lxly_213:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_211
    movq    -80(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_214
    cmpl    $1, (%rsi)
    je      .Lxly_214
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_215
.Lxly_214:
    call    xly_add
.Lxly_215:
    movq    %rax, -80(%rbp)
    movq    -88(%rbp), %rax
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
    je      .Lxly_216
    cmpl    $1, (%rsi)
    je      .Lxly_216
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_217
.Lxly_216:
    call    xly_add
.Lxly_217:
    movq    %rax, -88(%rbp)
    jmp     .Lxly_210
.Lxly_211:
    movq    -40(%rbp), %rax
    movq    %rax, -96(%rbp)
    subq    $16, %rsp
    movq    -96(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -104(%rbp)
.Lxly_218:
    movq    -104(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_220
    cmpl    $0, (%rsi)
    jne     .Lxly_220
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_221
.Lxly_220:
    call    xly_lt
.Lxly_221:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_219
    movq    -96(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_222
    cmpl    $1, (%rsi)
    je      .Lxly_222
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_223
.Lxly_222:
    call    xly_add
.Lxly_223:
    movq    %rax, -96(%rbp)
    movq    -104(%rbp), %rax
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
    je      .Lxly_224
    cmpl    $1, (%rsi)
    je      .Lxly_224
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_225
.Lxly_224:
    call    xly_add
.Lxly_225:
    movq    %rax, -104(%rbp)
    jmp     .Lxly_218
.Lxly_219:
    subq    $16, %rsp
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_238
    cmpl    $1, (%rsi)
    je      .Lxly_238
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_239
.Lxly_238:
    call    xly_add
.Lxly_239:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_236
    cmpl    $1, (%rsi)
    je      .Lxly_236
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_237
.Lxly_236:
    call    xly_add
.Lxly_237:
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_234
    cmpl    $1, (%rsi)
    je      .Lxly_234
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_235
.Lxly_234:
    call    xly_add
.Lxly_235:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_232
    cmpl    $1, (%rsi)
    je      .Lxly_232
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_233
.Lxly_232:
    call    xly_add
.Lxly_233:
    pushq   %rax
    movq    -80(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_230
    cmpl    $1, (%rsi)
    je      .Lxly_230
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_231
.Lxly_230:
    call    xly_add
.Lxly_231:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_228
    cmpl    $1, (%rsi)
    je      .Lxly_228
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_229
.Lxly_228:
    call    xly_add
.Lxly_229:
    pushq   %rax
    movq    -96(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_226
    cmpl    $1, (%rsi)
    je      .Lxly_226
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_227
.Lxly_226:
    call    xly_add
.Lxly_227:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.Lxly_fn_drawTableRow:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $192, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    %rdx, -24(%rbp)
    movq    %rcx, -32(%rbp)
    movq    %r8, -40(%rbp)
    movq    %r9, -184(%rbp)
    subq    $16, %rsp
    movq    -8(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    xly_null
    pushq   %rax
    subq    $8, %rsp
    movabsq $4607182418800017408, %rax
    movq    %rax, (%rsp)
    movsd   (%rsp), %xmm0
    addq    $8, %rsp
    call    xly_num
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_242
    cmpl    $0, (%rsi)
    jne     .Lxly_242
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_243
.Lxly_242:
    call    xly_eq
.Lxly_243:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_240
    call    .Lxly_fn_xuiBgBrightBlue
    call    .Lxly_fn_xuiFgBrightWhite
    call    .Lxly_fn_xuiBold
    jmp     .Lxly_241
.Lxly_240:
.Lxly_241:
    call    xly_null
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
    jne     .Lxly_246
    cmpl    $0, (%rsi)
    jne     .Lxly_246
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    sete    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_247
.Lxly_246:
    call    xly_eq
.Lxly_247:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_244
    call    .Lxly_fn_xuiFgWhite
    jmp     .Lxly_245
.Lxly_244:
.Lxly_245:
    movq    -16(%rbp), %rax
    movq    %rax, -48(%rbp)
    subq    $16, %rsp
    movq    -48(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -56(%rbp)
.Lxly_248:
    movq    -56(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_250
    cmpl    $0, (%rsi)
    jne     .Lxly_250
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_251
.Lxly_250:
    call    xly_lt
.Lxly_251:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_249
    movq    -48(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_252
    cmpl    $1, (%rsi)
    je      .Lxly_252
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_253
.Lxly_252:
    call    xly_add
.Lxly_253:
    movq    %rax, -48(%rbp)
    movq    -56(%rbp), %rax
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
    je      .Lxly_254
    cmpl    $1, (%rsi)
    je      .Lxly_254
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_255
.Lxly_254:
    call    xly_add
.Lxly_255:
    movq    %rax, -56(%rbp)
    jmp     .Lxly_248
.Lxly_249:
    movq    -24(%rbp), %rax
    movq    %rax, -64(%rbp)
    subq    $16, %rsp
    movq    -64(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -72(%rbp)
.Lxly_256:
    movq    -72(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_258
    cmpl    $0, (%rsi)
    jne     .Lxly_258
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_259
.Lxly_258:
    call    xly_lt
.Lxly_259:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_257
    movq    -64(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_260
    cmpl    $1, (%rsi)
    je      .Lxly_260
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_261
.Lxly_260:
    call    xly_add
.Lxly_261:
    movq    %rax, -64(%rbp)
    movq    -72(%rbp), %rax
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
    je      .Lxly_262
    cmpl    $1, (%rsi)
    je      .Lxly_262
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_263
.Lxly_262:
    call    xly_add
.Lxly_263:
    movq    %rax, -72(%rbp)
    jmp     .Lxly_256
.Lxly_257:
    movq    -32(%rbp), %rax
    movq    %rax, -80(%rbp)
    subq    $16, %rsp
    movq    -80(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -88(%rbp)
.Lxly_264:
    movq    -88(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_266
    cmpl    $0, (%rsi)
    jne     .Lxly_266
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_267
.Lxly_266:
    call    xly_lt
.Lxly_267:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_265
    movq    -80(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_268
    cmpl    $1, (%rsi)
    je      .Lxly_268
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_269
.Lxly_268:
    call    xly_add
.Lxly_269:
    movq    %rax, -80(%rbp)
    movq    -88(%rbp), %rax
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
    je      .Lxly_270
    cmpl    $1, (%rsi)
    je      .Lxly_270
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_271
.Lxly_270:
    call    xly_add
.Lxly_271:
    movq    %rax, -88(%rbp)
    jmp     .Lxly_264
.Lxly_265:
    movq    -40(%rbp), %rax
    movq    %rax, -96(%rbp)
    subq    $16, %rsp
    movq    -96(%rbp), %rax
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_78(%rip), %rdi
    leaq    .Lxly_str_79(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    movq    %rax, -104(%rbp)
.Lxly_272:
    movq    -104(%rbp), %rax
    pushq   %rax
    movq    -184(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $0, (%rdi)
    jne     .Lxly_274
    cmpl    $0, (%rsi)
    jne     .Lxly_274
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    ucomisd %xmm1, %xmm0
    setb    %al
    movzbl  %al, %edi
    call    xly_bool
    jmp     .Lxly_275
.Lxly_274:
    call    xly_lt
.Lxly_275:
    movq    %rax, %rdi
    call    xly_truthy
    testl   %eax, %eax
    jz      .Lxly_273
    movq    -96(%rbp), %rax
    pushq   %rax
    leaq    .Lxly_str_82(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_276
    cmpl    $1, (%rsi)
    je      .Lxly_276
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_277
.Lxly_276:
    call    xly_add
.Lxly_277:
    movq    %rax, -96(%rbp)
    movq    -104(%rbp), %rax
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
    je      .Lxly_278
    cmpl    $1, (%rsi)
    je      .Lxly_278
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_279
.Lxly_278:
    call    xly_add
.Lxly_279:
    movq    %rax, -104(%rbp)
    jmp     .Lxly_272
.Lxly_273:
    subq    $16, %rsp
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    pushq   %rax
    movq    -48(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_292
    cmpl    $1, (%rsi)
    je      .Lxly_292
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_293
.Lxly_292:
    call    xly_add
.Lxly_293:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_290
    cmpl    $1, (%rsi)
    je      .Lxly_290
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_291
.Lxly_290:
    call    xly_add
.Lxly_291:
    pushq   %rax
    movq    -64(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_288
    cmpl    $1, (%rsi)
    je      .Lxly_288
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_289
.Lxly_288:
    call    xly_add
.Lxly_289:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_286
    cmpl    $1, (%rsi)
    je      .Lxly_286
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_287
.Lxly_286:
    call    xly_add
.Lxly_287:
    pushq   %rax
    movq    -80(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_284
    cmpl    $1, (%rsi)
    je      .Lxly_284
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_285
.Lxly_284:
    call    xly_add
.Lxly_285:
    pushq   %rax
    leaq    .Lxly_str_10(%rip), %rdi
    call    xly_str
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_282
    cmpl    $1, (%rsi)
    je      .Lxly_282
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_283
.Lxly_282:
    call    xly_add
.Lxly_283:
    pushq   %rax
    movq    -96(%rbp), %rax
    movq    %rax, %rsi
    popq    %rdi
    cmpl    $1, (%rdi)
    je      .Lxly_280
    cmpl    $1, (%rsi)
    je      .Lxly_280
    movsd   8(%rdi), %xmm0
    movsd   8(%rsi), %xmm1
    addsd   %xmm1, %xmm0
    call    xly_num
    jmp     .Lxly_281
.Lxly_280:
    call    xly_add
.Lxly_281:
    movq    %rax, 0(%rsp)
    movq    %rsp, %rdx
    leaq    .Lxly_str_8(%rip), %rdi
    leaq    .Lxly_str_9(%rip), %rsi
    movl    $1, %ecx
    call    xly_call_module
    addq    $16, %rsp
    call    .Lxly_fn_xuiReset
    call    xly_null
    movq    %rbp, %rsp
    popq    %rbp
    ret

.section .rodata
.Lxly_str_0:
    .asciz  "Xenly Linux Dashboard"
.Lxly_str_1:
    .asciz  "v2.0.0"
.Lxly_str_2:
    .asciz  "os"
.Lxly_str_3:
    .asciz  "platform"
.Lxly_str_4:
    .asciz  "array"
.Lxly_str_5:
    .asciz  "create"
.Lxly_str_6:
    .asciz  "set"
.Lxly_str_7:
    .asciz  ""
.Lxly_str_8:
    .asciz  "io"
.Lxly_str_9:
    .asciz  "write"
.Lxly_str_10:
    .asciz  "  "
.Lxly_str_11:
    .asciz  "  \342\224\202  Platform: "
.Lxly_str_12:
    .asciz  "System Resources"
.Lxly_str_13:
    .asciz  "CPU Usage  "
.Lxly_str_14:
    .asciz  "%"
.Lxly_str_15:
    .asciz  "Memory     "
.Lxly_str_16:
    .asciz  " / "
.Lxly_str_17:
    .asciz  " MB"
.Lxly_str_18:
    .asciz  "Disk       "
.Lxly_str_19:
    .asciz  " GB"
.Lxly_str_20:
    .asciz  "Net Down   "
.Lxly_str_21:
    .asciz  " Mbps"
.Lxly_str_22:
    .asciz  "Net Up     "
.Lxly_str_23:
    .asciz  "CPU History (16 samples)"
.Lxly_str_24:
    .asciz  "0%"
.Lxly_str_25:
    .asciz  "\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200 last 16 ticks \342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200"
.Lxly_str_26:
    .asciz  "Peak: 82%"
.Lxly_str_27:
    .asciz  "Current: "
.Lxly_str_28:
    .asciz  "Services"
.Lxly_str_29:
    .asciz  "Web Server (nginx)"
.Lxly_str_30:
    .asciz  "Database  (postgres)"
.Lxly_str_31:
    .asciz  "Cache     (redis)"
.Lxly_str_32:
    .asciz  "Monitoring (prometheus)"
.Lxly_str_33:
    .asciz  "Running: "
.Lxly_str_34:
    .asciz  "/4"
.Lxly_str_35:
    .asciz  "success"
.Lxly_str_36:
    .asciz  "warn"
.Lxly_str_37:
    .asciz  "error"
.Lxly_str_38:
    .asciz  "Configuration Flags"
.Lxly_str_39:
    .asciz  "Dark Mode"
.Lxly_str_40:
    .asciz  "Notifications"
.Lxly_str_41:
    .asciz  "Auto Update"
.Lxly_str_42:
    .asciz  "Platform: "
.Lxly_str_43:
    .asciz  "Top Processes"
.Lxly_str_44:
    .asciz  "Process"
.Lxly_str_45:
    .asciz  "PID"
.Lxly_str_46:
    .asciz  "CPU%"
.Lxly_str_47:
    .asciz  "Memory"
.Lxly_str_48:
    .asciz  "  \342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200  \342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200  \342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200  \342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200\342\224\200"
.Lxly_str_49:
    .asciz  "nginx"
.Lxly_str_50:
    .asciz  "1024"
.Lxly_str_51:
    .asciz  "2.1%"
.Lxly_str_52:
    .asciz  "128 MB"
.Lxly_str_53:
    .asciz  "postgres"
.Lxly_str_54:
    .asciz  "1281"
.Lxly_str_55:
    .asciz  "18.4%"
.Lxly_str_56:
    .asciz  "2.1 GB"
.Lxly_str_57:
    .asciz  "xenly"
.Lxly_str_58:
    .asciz  "4096"
.Lxly_str_59:
    .asciz  "34.2%"
.Lxly_str_60:
    .asciz  "512 MB"
.Lxly_str_61:
    .asciz  "redis"
.Lxly_str_62:
    .asciz  "1890"
.Lxly_str_63:
    .asciz  "0.8%"
.Lxly_str_64:
    .asciz  "256 MB"
.Lxly_str_65:
    .asciz  "prometheus"
.Lxly_str_66:
    .asciz  "2001"
.Lxly_str_67:
    .asciz  "4.1%"
.Lxly_str_68:
    .asciz  "180 MB"
.Lxly_str_69:
    .asciz  "Backup completed successfully \342\200\224 3.2 GB archived"
.Lxly_str_70:
    .asciz  "Redis cache miss rate above threshold (12.3%)"
.Lxly_str_71:
    .asciz  "Xenly 2.0.0 release build finished in 0.3s"
.Lxly_str_72:
    .asciz  "info"
.Lxly_str_73:
    .asciz  "Disk partition /dev/sda1 at 62% capacity"
.Lxly_str_74:
    .asciz  "  q: quit   r: refresh   s: services   h: help   \342\224\202  Declarative Linux UI"
.Lxly_str_75:
    .asciz  "\n"
.Lxly_str_76:
    .asciz  ";"
.Lxly_str_77:
    .asciz  "H"
.Lxly_str_78:
    .asciz  "string"
.Lxly_str_79:
    .asciz  "len"
.Lxly_str_80:
    .asciz  "\342\225\224"
.Lxly_str_81:
    .asciz  "\342\225\220"
.Lxly_str_82:
    .asciz  " "
.Lxly_str_83:
    .asciz  "\342\225\227"
.Lxly_str_84:
    .asciz  "\342\225\221"
.Lxly_str_85:
    .asciz  "\342\225\232"
.Lxly_str_86:
    .asciz  "\342\225\235"
.Lxly_str_87:
    .asciz  "["
.Lxly_str_88:
    .asciz  "\342\226\210"
.Lxly_str_89:
    .asciz  "\342\226\221"
.Lxly_str_90:
    .asciz  "] "
.Lxly_str_91:
    .asciz  "neutral"
.Lxly_str_92:
    .asciz  "\342\226\201"
.Lxly_str_93:
    .asciz  "\342\226\202"
.Lxly_str_94:
    .asciz  "\342\226\203"
.Lxly_str_95:
    .asciz  "\342\226\204"
.Lxly_str_96:
    .asciz  "\342\226\205"
.Lxly_str_97:
    .asciz  "\342\226\206"
.Lxly_str_98:
    .asciz  "\342\226\207"
.Lxly_str_99:
    .asciz  "get"
.Lxly_str_100:
    .asciz  "[\342\234\223] "
.Lxly_str_101:
    .asciz  "[ ] "
.Lxly_str_102:
    .asciz  ": "
.Lxly_str_103:
    .asciz  "\342\234\224  "
.Lxly_str_104:
    .asciz  "\342\234\226  "
.Lxly_str_105:
    .asciz  "\342\204\271  "
.Lxly_str_106:
    .asciz  "\342\232\240  "

.section .note.GNU-stack,"",@progbits
