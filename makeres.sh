
mkdir -p res/out;
cd res/in/;
for f in *; do
    bf=$(basename -- $f);
    ff="${bf%.*}";
    echo "Processing file $ff..";
    objcopy -B i386:x86-64 -I binary -O elf64-x86-64 $bf ../out/$ff.o;
done
cd ../../;

