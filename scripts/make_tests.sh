test_dir="./test/instruction"

for i in $(ls test/instruction/*.s) ; do 
    riscv64-unknown-elf-gcc -Ttext=0x10000 -Tdata=0x11100 \
        -Wall -march=rv64imafd -O0 \
        -nostdlib -fno-builtin -nodefaultlibs -o ${test_dir}/$(basename $i .s).bin $i
    touch ${test_dir}/$(basename $i .s).conf
done