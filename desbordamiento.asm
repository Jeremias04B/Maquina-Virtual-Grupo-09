mov eax, 50000
mov ebx, 70000
add eax, ebx
mov [ds], eax
mov edx, ds
mov eax, 0x01
ldh ecx, 0x04
ldl ecx,0x01
sys 0x02
stop