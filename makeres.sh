
mkdir -p res/out
cd res/in/
for f in *; do
    bf=$(basename -- $f);
    ff="${bf%.*}"
    echo "Processing file $ff.."
    if [ "$1" = "-cross" ]; then
        x86_64-w64-mingw32-ld -r -b binary -o ../out/$ff.o $bf
    else
        ld -r -b binary -o ../out/$ff.o $bf
    fi
done
cd ../../

